/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
#include <arduino.h>
#include "vt_cs_signature_features.h"
#include "vt_cs_api.h"
#include "vt_cs_database.h"
#include "vt_cs_raw_signature_read.h"
#include "vt_cs_sensor_status.h"
#include <math.h>
#include "vt_debug.h"

static VT_VOID cs_sensor_status_with_non_repeating_signature_template(VT_CURRENTSENSE_OBJECT* cs_object)
{
    static VT_FLOAT raw_signature[VT_CS_SAMPLE_LENGTH];
    memset(raw_signature,0,sizeof(VT_FLOAT)*VT_CS_SAMPLE_LENGTH);

    VT_FLOAT sampling_frequency                 = 0;
    VT_UINT num_datapoints                      = 0;
    VT_FLOAT avg_curr_on;
    VT_FLOAT avg_curr_on_saved;
    VT_FLOAT avg_curr_off;
    VT_FLOAT avg_curr_off_saved;
    VT_FLOAT avg_curr_off_drift;
    VT_FLOAT avg_curr_on_drift;

    VT_FLOAT avg_curr_drift = 0;

    if (cs_fetch_template_non_repeating_signature_average_current(cs_object, &avg_curr_on_saved, &avg_curr_off_saved) ==
        VT_SUCCESS)
    {
        if (cs_non_repeating_raw_signature_fetch_stored_current_measurement(
                cs_object, raw_signature, &sampling_frequency, &num_datapoints) == VT_SUCCESS)
        {

            if (cs_non_repeating_signature_average_current_compute(
                    cs_object, raw_signature, num_datapoints, &avg_curr_on, &avg_curr_off) == VT_SUCCESS)
            {
                avg_curr_drift = cs_non_repeating_signature_average_current_evaluate(
                    avg_curr_on, avg_curr_on_saved, avg_curr_off, avg_curr_off_saved,&avg_curr_off_drift,&avg_curr_on_drift);

                if (avg_curr_drift > VT_CS_MAX_AVG_CURR_DRIFT)
                {   VTLogDebugNoTag("\n non_repeating_signature -  VT_SIGNATURE_NOT_MATCHING \n");
                    //cs_object->sensor_status = VT_SIGNATURE_NOT_MATCHING;
                    cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=VT_SIGNATURE_NOT_MATCHING;
                    cs_object->sensor_drift  = avg_curr_drift;
                    cs_object->fingerprintdb.debug_template.non_rep_current_drift=avg_curr_drift;
                }
                else
                {   VTLogDebugNoTag("\n non_repeating_signature -  VT_SIGNATURE_MATCHING \n");
                    //cs_object->sensor_status = VT_SIGNATURE_MATCHING;
                    cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=VT_SIGNATURE_MATCHING;
                    cs_object->sensor_drift  = avg_curr_drift;
                    cs_object->fingerprintdb.debug_template.non_rep_current_drift=avg_curr_drift;
                }
                return;
            }
            //cs_object->sensor_status = VT_SIGNATURE_COMPUTE_FAIL;
            VTLogDebugNoTag("\n non_repeating_signature -  VT_SIGNATURE_COMPUTE_FAIL \n");
            cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=VT_SIGNATURE_COMPUTE_FAIL;
            cs_object->sensor_drift  = 100;
            cs_object->fingerprintdb.debug_template.non_rep_current_drift=100;
        }
        //cs_object->sensor_status = VT_SIGNATURE_COMPUTE_FAIL;
        VTLogDebugNoTag("\n non_repeating_signature -  VT_SIGNATURE_COMPUTE_FAIL \n");
        cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=VT_SIGNATURE_COMPUTE_FAIL;
        cs_object->sensor_drift  = 100;
        cs_object->fingerprintdb.debug_template.non_rep_current_drift=100;
    }

    //cs_object->sensor_status = VT_SIGNATURE_DB_EMPTY;
    VTLogDebugNoTag("\n non_repeating_signature -  VT_SIGNATURE_DB_EMPTY \n");
    cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=VT_SIGNATURE_DB_EMPTY;
    cs_object->sensor_drift  = 100;
    cs_object->fingerprintdb.debug_template.non_rep_current_drift=100;
}

static VT_VOID cs_sensor_status_with_repeating_signature_template(VT_CURRENTSENSE_OBJECT* cs_object)
{
    #if VT_LOG_LEVEL > 2
    //Serial.printf(" \n inside rep sig evaluate \n ");
    #endif

    static VT_FLOAT raw_signature[VT_CS_SAMPLE_LENGTH];
    memset(raw_signature,0,sizeof(VT_FLOAT)*VT_CS_SAMPLE_LENGTH);

    VT_FLOAT lowest_sample_freq_saved;
    VT_FLOAT offset_current;
    VT_FLOAT offset_current_saved;

    VT_FLOAT sampling_frequency_saved;
    VT_FLOAT signature_frequency[VT_CS_ACRFFT_MAX_PEAKS];
    VT_FLOAT duty_cycle;
    VT_FLOAT relative_current_draw;
    VT_FLOAT signature_frequency_saved;
    VT_FLOAT sec_sig_freq;
    VT_FLOAT duty_cycle_saved;
    VT_FLOAT relative_current_draw_saved;
    VT_FLOAT average_current_draw;
    VT_FLOAT average_current_draw_saved;

    VT_FLOAT offset_current_drift = 0;
    VT_FLOAT feature_vector_drift = 0;
    VT_FLOAT temp_feature_vector_drift = 0;
    VT_FLOAT temp_relative_current_drift = 0;
    VT_FLOAT temp_offset_current_drift = 0;

    VT_FLOAT current_cluster_1_standby;
    VT_FLOAT current_cluster_2_active;
    VT_FLOAT current_average;

    VT_BOOL offset_current_unavailable = false;

    VT_UINT signatures_evaluated                  = 0;
    VT_BOOL signature_feature_vector_compute_fail = false;
    VT_BOOL signature_offset_current_compute_fail = false;

#if VT_LOG_LEVEL > 2
    VT_INT decimal=0;
    VT_FLOAT frac_float;
    VT_INT frac;
    #endif

    VT_BOOL sensor_status=true;
    VT_BOOL temp_sensor_status=true;
    VT_BOOL offset_curr_status=false;

    if (cs_fetch_template_repeating_signature_offset_current(cs_object, &lowest_sample_freq_saved, &offset_current_saved))
    {
        offset_current_unavailable = true;
        
        #if VT_LOG_LEVEL > 2
        Serial.printf("\n offset_current_unavailable\n");
        #endif
    }

    #if VT_LOG_LEVEL > 3
            decimal    = lowest_sample_freq_saved;
        frac_float = lowest_sample_freq_saved - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
    
    VTLogDebugNoTag("\nLOWEST TEMPLATE FREQ: %d.%04d : \n", decimal, frac);
    #endif
    if (!offset_current_unavailable)
    {   

        cs_object->fingerprintdb.debug_template.offset_current_drift_valid=true;

        if (cs_repeating_raw_signature_fetch_stored_current_measurement(
                cs_object, raw_signature, lowest_sample_freq_saved, VT_CS_SAMPLE_LENGTH) == VT_SUCCESS)
        {   
            if (cs_repeating_signature_offset_current_compute(cs_object, raw_signature, VT_CS_SAMPLE_LENGTH, &offset_current) ==
                VT_SUCCESS)
            {   
                temp_offset_current_drift = cs_repeating_signature_offset_current_evaluate(offset_current, offset_current_saved);
                if((temp_offset_current_drift)<40){
                    
                    offset_curr_status=true;
                    offset_current_drift=temp_offset_current_drift*(50/40);
                    cs_object->fingerprintdb.debug_template.offset_current_drift=offset_current_drift;
                }
                else{
                    
                    offset_curr_status=false;
                    offset_current_drift=temp_offset_current_drift*(50/40);
                    cs_object->fingerprintdb.debug_template.offset_current_drift=offset_current_drift;
                }
            }
            else
            {   
                signature_offset_current_compute_fail = true;
                cs_object->fingerprintdb.debug_template.offset_current_drift=100;
            }
        }
        else
        {
            signature_offset_current_compute_fail = true;
            cs_object->fingerprintdb.debug_template.offset_current_drift=100;
        }
    }
    else
    {
        cs_object->fingerprintdb.debug_template.offset_current_drift_valid=false;
    }

    if (!signature_offset_current_compute_fail)
        {sensor_status=sensor_status && offset_curr_status;}
    else
      {
        #if VT_LOG_LEVEL > 3
        Serial.printf("\n signature_offset_current_compute_fail\n");
        #endif
      }  
    
    for (VT_UINT iter = 0; iter < cs_object->fingerprintdb.template_struct.repeating_signatures.num_signatures; iter++)

    {
        signature_feature_vector_compute_fail=false;
    temp_feature_vector_drift = 0;
        #if VT_LOG_LEVEL > 3
    VTLogDebugNoTag("\n sensor_status : %d",sensor_status);
    #endif
        if (cs_fetch_template_repeating_signature_feature_vector(cs_object,
                iter,
                &sampling_frequency_saved,
                &signature_frequency_saved,
                &duty_cycle_saved,
                &relative_current_draw_saved,
                &average_current_draw_saved,
                &sec_sig_freq))
        {
            break;
        }

        if (cs_repeating_raw_signature_fetch_stored_current_measurement(
                cs_object, raw_signature, sampling_frequency_saved, VT_CS_SAMPLE_LENGTH))
        {
            signature_feature_vector_compute_fail = true;
            break;
        }

        cs_object->fingerprintdb.debug_template.rep_sampling_freq[iter]=sampling_frequency_saved;

        #if VT_LOG_LEVEL > 2
        decimal    = sampling_frequency_saved;
        frac_float = sampling_frequency_saved - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        Serial.printf("\ncalculating for %d.%04d : \n", decimal, frac);

                decimal    = sec_sig_freq;
        frac_float = sec_sig_freq - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        Serial.printf("\nsec sig freq %d.%04d : \n", decimal, frac);

         Serial.printf("\nRAW SIG:\n");
   
        for (VT_INT iter1 = 0; iter1 < VT_CS_SAMPLE_LENGTH; iter1++)
            {
                decimal    = raw_signature[iter1];
                frac_float = raw_signature[iter1] - (VT_FLOAT)decimal;
                frac       = fabsf(frac_float) * 10000;
                Serial.printf("%d.%04d, ", decimal, frac);
            }
        Serial.printf("\n");
        #endif
    
        if (cs_repeating_signature_feature_vector_compute(cs_object,
                raw_signature,
                VT_CS_SAMPLE_LENGTH,
                sampling_frequency_saved,
                signature_frequency,
                &duty_cycle,
                &relative_current_draw,
                &current_cluster_1_standby,
                &current_cluster_2_active,
                &current_average))
        {
            signature_feature_vector_compute_fail = true;
           // break;
        }
    /*
    if (sampling_frequency_saved==VT_CS_ADC_MAX_SAMPLING_FREQ){
        if ((signature_feature_vector_compute_fail==false))
        {

            //temp_relative_current_drift=cs_repeating_signature_relative_current_evaluate(relative_current_draw,relative_current_draw_saved);

            //if (temp_relative_current_drift<60.0f){
                
            if(cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq_and_duty_cycle_valid==true)
            {
                temp_feature_vector_drift = cs_repeating_signature_feature_vector_evaluate(signature_frequency,
                signature_frequency_saved,
                duty_cycle,
                duty_cycle_saved,
                relative_current_draw,
                relative_current_draw_saved);
            }
        }
                        if (signature_feature_vector_compute_fail==true){
                temp_feature_vector_drift =100;
            }
            //}

            // else{signature_feature_vector_compute_fail=true;}

            // }

            // if ((signature_feature_vector_compute_fail==true) || (temp_feature_vector_drift>50)){
            //     temp_sensor_status=false;
            //     temp_feature_vector_drift=100;
            //     }
            // else {
            //     temp_sensor_status=true;
                               
            // }
            if(cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_dif_valid==true)
            {
                temp_relative_current_drift=cs_repeating_signature_relative_current_evaluate(relative_current_draw,relative_current_draw_saved);
            }

            if((cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq_and_duty_cycle_valid==true)&&
                    (cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_dif_valid==true))
            {   
                Serial.printf(" \n both signature_freq_and_duty_cycle_valid & current_dif_valid valid  \n");
                if((temp_feature_vector_drift<50.0f) || (temp_relative_current_drift<55.0f))
                {
                    temp_sensor_status=true;
                    feature_vector_drift=feature_vector_drift+temp_feature_vector_drift+temp_relative_current_drift;
                }
                else
                    temp_sensor_status=false;
            }
            else if(cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq_and_duty_cycle_valid==true)
            {
                Serial.printf(" \n only signature_freq_and_duty_cycle_valid  \n");
                if(temp_feature_vector_drift<50.0f)
                {
                    temp_sensor_status=true;
                    feature_vector_drift=feature_vector_drift+temp_feature_vector_drift;
                }
                else
                    temp_sensor_status=false;
            }
            else if(cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_dif_valid==true)
            {
                Serial.printf(" \n only current_dif_valid  \n");
                if(temp_relative_current_drift<55.0f)
                {
                    temp_sensor_status=true;
                    feature_vector_drift=feature_vector_drift+temp_relative_current_drift;
                }
                else
                    temp_sensor_status=false;
            }
            else
                {
                    Serial.printf(" \n NONE valid  \n"); 
                    temp_sensor_status=false;
                }
            
            //            if ((temp_feature_vector_drift<50.0f) || (temp_relative_current_drift<55.0f)){
            //    
            //        #if VT_LOG_LEVEL > 3
            //        VTLogDebugNoTag("component correct");
            //        #endif
            //    temp_sensor_status=true;
            //}
            //else{
            //        #if VT_LOG_LEVEL > 3
            //        VTLogDebugNoTag("component faulty");
            //        #endif
            //    temp_sensor_status=false;                
            //}
            //feature_vector_drift=feature_vector_drift +temp_feature_vector_drift;
            
            sensor_status=sensor_status && temp_sensor_status;
            
        signatures_evaluated++;}

    else {
            */
            if ((signature_feature_vector_compute_fail==false))
            {
            if(cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq_and_duty_cycle_valid==true)
            {

            temp_feature_vector_drift = cs_repeating_signature_feature_vector_evaluate(signature_frequency,
            signature_frequency_saved,
            duty_cycle,
            duty_cycle_saved,
            relative_current_draw,
            relative_current_draw_saved,
            sec_sig_freq);
            }
            cs_object->fingerprintdb.debug_template.rep_feature_vector_drift[iter]=temp_feature_vector_drift;
            cs_object->fingerprintdb.debug_template.rep_feature_vector_drift_valid[iter]=true;

            }
            if (signature_feature_vector_compute_fail==true){
                temp_feature_vector_drift =100;
                cs_object->fingerprintdb.debug_template.rep_feature_vector_drift_valid[iter]=false;
                cs_object->fingerprintdb.debug_template.rep_feature_vector_drift[iter]=100;
            }

            if(cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_dif_valid==true)
            {
            temp_relative_current_drift=cs_repeating_signature_relative_current_evaluate(relative_current_draw,relative_current_draw_saved,average_current_draw,average_current_draw_saved);
            cs_object->fingerprintdb.debug_template.rep_relative_current_drift_valid[iter]=true;
            cs_object->fingerprintdb.debug_template.rep_relative_current_drift[iter]=temp_relative_current_drift;
            }
            else
            {            
            cs_object->fingerprintdb.debug_template.rep_relative_current_drift_valid[iter]=false;
            cs_object->fingerprintdb.debug_template.rep_relative_current_drift[iter]=100;
            }

            if((cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq_and_duty_cycle_valid==true)&&
                    (cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_dif_valid==true))
            {   
                #if VT_LOG_LEVEL > 3
                Serial.printf(" \n both signature_freq_and_duty_cycle_valid & current_dif_valid valid  \n");
                #endif
                if((temp_feature_vector_drift<50.0f) && (temp_relative_current_drift<50.0f))
                {
                    temp_sensor_status=true;
                    feature_vector_drift=feature_vector_drift+(temp_feature_vector_drift+temp_relative_current_drift)/2;
                }
                else
                    temp_sensor_status=false;
            }
            else if(cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq_and_duty_cycle_valid==true)
            {
                #if VT_LOG_LEVEL > 3
                Serial.printf(" \n only signature_freq_and_duty_cycle_valid  \n");
                #endif
                if(temp_feature_vector_drift<50.0f)
                {
                    temp_sensor_status=true;
                    feature_vector_drift=feature_vector_drift+temp_feature_vector_drift;
                }
                else
                    temp_sensor_status=false;
            }
            else if(cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_dif_valid==true)
            {
                #if VT_LOG_LEVEL > 3
                Serial.printf(" \n only current_dif_valid  \n");
                #endif
                if(temp_relative_current_drift<50.0f)
                {
                    temp_sensor_status=true;
                    feature_vector_drift=feature_vector_drift+temp_relative_current_drift;
                }
                else
                    temp_sensor_status=false;
            }
            else
                {
                    #if VT_LOG_LEVEL > 3
                    Serial.printf(" \n NONE valid  \n"); 
                    #endif
                    temp_sensor_status=false;
                }


            /*
            if ((temp_feature_vector_drift<50.0f) || (temp_relative_current_drift<40.0f)){
                
                    #if VT_LOG_LEVEL > 3
                    VTLogDebugNoTag("component correct");
                    #endif
                temp_sensor_status=true;
            }
            else{
                    #if VT_LOG_LEVEL > 3
                    VTLogDebugNoTag("component faulty");
                    #endif
                temp_sensor_status=false;
            }
            */
            sensor_status=sensor_status && temp_sensor_status;
            //Serial.printf("sensor_status : %d",sensor_status);
            /*
            if(temp_feature_vector_drift<(temp_relative_current_drift*(50/30))){
                feature_vector_drift=feature_vector_drift +temp_feature_vector_drift;
            }
            else{
                feature_vector_drift=feature_vector_drift +(temp_relative_current_drift*(50/30));
            }
            */
            signatures_evaluated++;  
            
            //}

    



    }
    Serial.println("");
    if (signatures_evaluated)
    {
        feature_vector_drift /= signatures_evaluated;
    }

    if (signature_offset_current_compute_fail)
    {
        //cs_object->sensor_status = VT_SIGNATURE_COMPUTE_FAIL;
        VTLogDebugNoTag("\n repeating_signature -  VT_SIGNATURE_COMPUTE_FAIL \n");
        cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status=VT_SIGNATURE_COMPUTE_FAIL;
        cs_object->sensor_drift  = 100;
    }

    

    else if ((signatures_evaluated == 0) && offset_current_unavailable)
    {
        //cs_object->sensor_status = VT_SIGNATURE_DB_EMPTY;
        VTLogDebugNoTag("\n repeating_signature -  VT_SIGNATURE_DB_EMPTY \n");
        cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status=VT_SIGNATURE_DB_EMPTY;
        cs_object->sensor_drift  = 100;
    }
        cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status=!sensor_status;
        VTLogDebugNoTag("\n repeating_signature -  (!sensor_status), repeating_sensor_status : %d \n",cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status);
        //cs_object->sensor_status = !sensor_status;
        
        if (signatures_evaluated && (!offset_current_unavailable))
        {
            cs_object->sensor_drift = (offset_current_drift + feature_vector_drift) / 2;
        }
        else if (signatures_evaluated)
        {
            cs_object->sensor_drift = feature_vector_drift;
        }
        else
        {
            cs_object->sensor_drift = offset_current_drift;
        }
    
  
    return;
}


VT_VOID average_status(VT_CURRENTSENSE_OBJECT* cs_object){

    VT_UINT sum=0;

    for(int i=VT_HISTORY_ARRAY_LENGTH-1;i>0;i--){
        cs_object->hist_array[i]=cs_object->hist_array[i-1];
            }
    cs_object->hist_array[0]=cs_object->sensor_status;

    #if VT_LOG_LEVEL > 2
    Serial.print("HIST ARRAY: ");
    #endif

    for(int i=0;i<VT_HISTORY_ARRAY_LENGTH;i++){
        sum=sum+cs_object->hist_array[i];
        #if VT_LOG_LEVEL > 2
        Serial.printf("%d",cs_object->hist_array[i]);
        #endif
            }

    #if VT_LOG_LEVEL > 2
    Serial.print(" \n ");
    #endif

        #if VT_LOG_LEVEL > 3
        Serial.printf("\n%d",sum);
        #endif

    if(sum>=ceil((float)VT_HISTORY_ARRAY_LENGTH/2)){
        cs_object->sensor_status=true;
    }
    else{
        cs_object->sensor_status=false;
    }

    #if VT_LOG_LEVEL > 3
    Serial.printf("\nFINAL STATUS:%d\n",cs_object->sensor_status);
    #endif


}
VT_VOID cs_sensor_status(VT_CURRENTSENSE_OBJECT* cs_object)
{

        if(cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_valid==true)
            {
                cs_sensor_status_with_non_repeating_signature_template(cs_object);
                cs_object->fingerprintdb.debug_template.non_rep_drift_valid=true;
            }
        else 
            {cs_object->fingerprintdb.debug_template.non_rep_drift_valid=false;}
            
        if(cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_valid==true)
            {cs_sensor_status_with_repeating_signature_template(cs_object);}

        #if VT_LOG_LEVEL > 2
        Serial.printf("NON REPEATING signature Valid :  %s , ",(cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_valid?"true":"false"));
        #endif

        #if VT_LOG_LEVEL > 2
        Serial.printf("REPEATING signature Valid : %s\n",(cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_valid?"true":"false"));
        #endif

        #if VT_LOG_LEVEL > 2        
        Serial.printf("non_repeating_sensor_status : %s, ",(cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status?"false":"true"));
        Serial.printf("repeating_sensor_status : %s\n",(cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status?"false":"true"));
        #endif

        if(cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_valid==true || cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_valid==true)
        {
            cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status=!cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status;

            if(cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status){
                cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=0;
            }
            else{
                cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=1;
            }

            if(cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_valid==true && cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_valid==true){
                cs_object->sensor_status=
                cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status & cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status;

            }
            else{
            cs_object->sensor_status=
                cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status|cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status;
            }

            average_status(cs_object);
            cs_object->sensor_status=!cs_object->sensor_status;

            cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status=!cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status;

            if(cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status){
                cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=0;
            }
            else{
                cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=1;
            }
        }
        else if(cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_valid==true)
        {
            if(cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status){
                cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=0;
            }
            else{
                cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=1;
            }

            cs_object->sensor_status=cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status;

            average_status(cs_object);
            cs_object->sensor_status=!cs_object->sensor_status;

            if(cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status){
                cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=0;
            }
            else{
                cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status=1;
            }
        }
        else if(cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_valid==true)
        {
            cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status=!cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status;

            cs_object->sensor_status=
                cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status;

            average_status(cs_object);
            cs_object->sensor_status=!cs_object->sensor_status;

            cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status=!cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status;

        }
        else 
        {
            cs_object->sensor_status=VT_SIGNATURE_DB_EMPTY;
        }
        #if VT_LOG_LEVEL > 2        
        //Serial.printf("\n repeating_sensor_status : %s\n",(cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_sensor_status?"false":"true"));
        //Serial.printf("\n non_repeating_sensor_status : %s\n",(cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_sensor_status?"false":"true"));
        Serial.printf("cs_object->sensor_status : %s\n",(cs_object->sensor_status?"false":"true"));
        #endif
        //if either of them s true, output is true.
}
