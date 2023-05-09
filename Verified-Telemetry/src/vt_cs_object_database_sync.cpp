/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
#include <arduino.h>
#include "vt_cs_api.h"
#include "vt_debug.h"
#include "vt_cs_database.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
//try to print all the feilds just to be sure

VT_VOID vt_cs_repeating_signature_type(VT_CURRENTSENSE_OBJECT* cs_object)
{
    for(VT_UINT iter=0;iter<cs_object->fingerprintdb.template_struct.repeating_signatures.num_signatures;iter++)
    {
        if((cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq==0)&&
                (cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].duty_cycle==0))
        {   
            #if VT_LOG_LEVEL > 2
            Serial.printf(" \n signature_freq_and_duty_cycle_valid : false  \n");
            #endif

            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq_and_duty_cycle_valid=false;
        }
        else 
        {   
            #if VT_LOG_LEVEL > 2
            Serial.printf(" \n signature_freq_and_duty_cycle_valid : true  \n");
            #endif

            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq_and_duty_cycle_valid=true;
        }
        if(cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].relative_curr_draw==0)
        {   
            #if VT_LOG_LEVEL > 2
            Serial.printf(" \n current_dif_valid : false  \n");
            #endif

            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_dif_valid=false;
        }
        else 
        {
            #if VT_LOG_LEVEL > 2
            Serial.printf(" \n current_dif_valid : true  \n");
            #endif

            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_dif_valid=true;
        }
    }
}


VT_VOID vt_currentsense_object_database_sync(VT_CURRENTSENSE_OBJECT* cs_object, VT_CURRENTSENSE_DATABASE_FLATTENED* flattened_db)
{
    VT_CHAR* token;
    VT_CHAR* csvString;
    VT_UINT iter;

    VT_INT decimal;
    VT_FLOAT frac_float;
    VT_INT frac;

    cs_object->fingerprintdb.template_type = atof((VT_CHAR*)flattened_db->template_type);

    if(cs_object->fingerprintdb.template_type==VT_CS_BOTH_REPEATING_NONREPEATING_SIGNATURE_VALID)
        {
        cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_valid=true;
        cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_valid=true;
        }
    else if(cs_object->fingerprintdb.template_type==VT_CS_ONLY_REPEATING_SIGNATURE_VALID)
       { 
        cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_valid=false;
        cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_valid=true;
       }
    else if(cs_object->fingerprintdb.template_type==VT_CS_ONLY_NONREPEATING_SIGNATURE_VALID)
        {
        cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_valid=true;
        cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_valid=false;
        }
    else if(cs_object->fingerprintdb.template_type==VT_CS_NO_SIGNATURE_VALID)
        {
        cs_object->fingerprintdb.template_struct.non_repeating_signature.non_repeating_valid=false;
        cs_object->fingerprintdb.template_struct.repeating_signatures.repeating_valid=false;
        }
    #if VT_LOG_LEVEL > 2
    Serial.printf(" \n template_struct type - %d \n", (int)cs_object->fingerprintdb.template_type);
    #endif

        cs_object->fingerprintdb.template_struct.non_repeating_signature.avg_curr_off =
            atof((VT_CHAR*)flattened_db->non_repeating_signature_avg_curr_off);
            

        decimal    = cs_object->fingerprintdb.template_struct.non_repeating_signature.avg_curr_off;
        frac_float = cs_object->fingerprintdb.template_struct.non_repeating_signature.avg_curr_off - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        Serial.printf("\navg_curr_off: %d.%04d : \n", decimal, frac);
        #endif


        cs_object->fingerprintdb.template_struct.non_repeating_signature.avg_curr_on =
            atof((VT_CHAR*)flattened_db->non_repeating_signature_avg_curr_on);

        decimal    = cs_object->fingerprintdb.template_struct.non_repeating_signature.avg_curr_on;
        frac_float = cs_object->fingerprintdb.template_struct.non_repeating_signature.avg_curr_on - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        Serial.printf("\navg_curr_on: %d.%04d : \n", decimal, frac);
        #endif

        cs_object->fingerprintdb.template_struct.repeating_signatures.num_signatures =
            atof((VT_CHAR*)flattened_db->repeating_signature_num_signatures);

        #if VT_LOG_LEVEL > 2
        Serial.printf(" num_signatures : %d \n", (int)cs_object->fingerprintdb.template_struct.repeating_signatures.num_signatures);
        #endif

        cs_object->fingerprintdb.template_struct.repeating_signatures.offset_current =
            (VT_FLOAT)atof((VT_CHAR*)flattened_db->repeating_signature_offset_curr);

        decimal    = cs_object->fingerprintdb.template_struct.repeating_signatures.offset_current;
        frac_float = cs_object->fingerprintdb.template_struct.repeating_signatures.offset_current - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        Serial.printf("\n offset_current : %d.%04d : \n", decimal, frac);
        #endif

        cs_object->fingerprintdb.template_struct.repeating_signatures.lowest_sample_freq =
            (VT_FLOAT)atof((VT_CHAR*)flattened_db->repeating_signature_lowest_sample_freq);

        decimal    = cs_object->fingerprintdb.template_struct.repeating_signatures.lowest_sample_freq;
        frac_float = cs_object->fingerprintdb.template_struct.repeating_signatures.lowest_sample_freq - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        Serial.printf("\n lowest_sample_freq : %d.%04d : \n", decimal, frac);
        #endif
        float temp_value=0;
        //#if VT_LOG_LEVEL > 2
        //Serial.printf("repeating_signature_sampling_freq : \n");
        //#endif
        iter = 0;
        for (csvString = (VT_CHAR*)flattened_db->repeating_signature_sampling_freq;; csvString = NULL)
        {
            token = strtok(csvString, ",");
            if (token == NULL)
            {
                break;
            }
            #if VT_LOG_LEVEL > 2
            Serial.printf(" token : - %s \n", token);
            #endif
            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].sampling_freq = atof((VT_CHAR*)token);
            //if you only printcs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].sampling_freq 
            //the value comes out to be zero, somthing that float does 
            //if you assign it to a float as be low it works fine 
            temp_value=cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].sampling_freq;
            iter++;

        decimal    = temp_value;
        frac_float = temp_value - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        Serial.printf("\nrepeating_signature_sampling_freq: %d.%04d : \n", decimal, frac);
        #endif

        }
        #if VT_LOG_LEVEL > 2
        Serial.printf("repeating_signature_freq : \n");
        #endif
        iter = 0;
        for (csvString = (VT_CHAR*)flattened_db->repeating_signature_freq;; csvString = NULL)
        {
            token = strtok(csvString, ",");
            if (token == NULL)
            {
                break;
            }
            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq = atof((VT_CHAR*)token);
            temp_value=cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq;
            iter++;

        decimal    = temp_value;
        frac_float = temp_value - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        Serial.printf("\nrepeating_signature_freq: %d.%04d : \n", decimal, frac);
        #endif

        }
        //#if VT_LOG_LEVEL > 2
        //Serial.printf("repeating_sec_signature_freq : \n");
        //#endif

        iter = 0;
        for (csvString = (VT_CHAR*)flattened_db->repeating_sec_signature_freq;; csvString = NULL)
        {
            token = strtok(csvString, ",");
            if (token == NULL)
            {
                break;
            }
            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].sec_signature_freq = atof((VT_CHAR*)token);
            temp_value=cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].sec_signature_freq;
            iter++;

        decimal    = temp_value;
        frac_float = temp_value - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        printf("\nrepeating_sec_signature_freq: %d.%04d : \n", decimal, frac);
        #endif 

        #if VT_LOG_LEVEL > 2
        Serial.printf("relative_curr_draw : \n");
        #endif
        }
        
        iter = 0;
        for (csvString = (VT_CHAR*)flattened_db->repeating_signature_relative_curr_draw;; csvString = NULL)
        {
            token = strtok(csvString, ",");
            if (token == NULL)
            {
                break;
            }
            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].relative_curr_draw = atof((VT_CHAR*)token);
            temp_value=cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].relative_curr_draw;
            iter++;

        decimal    = temp_value;
        frac_float = temp_value - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        Serial.printf("\n relative_curr_draw: %d.%04d : \n", decimal, frac);
        #endif

        }
        //#if VT_LOG_LEVEL > 2
        //Serial.printf("current_cluster_1_standby : \n");
        //#endif
        iter = 0;
        for (csvString = (VT_CHAR*)flattened_db->repeating_signature_relative_curr_cluster_1_standby;; csvString = NULL)
        {
            token = strtok(csvString, ",");
            if (token == NULL)
            {
                break;
            }
            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_cluster_1_standby = atof((VT_CHAR*)token);
            temp_value=cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_cluster_1_standby;
            iter++;

        decimal    = temp_value;
        frac_float = temp_value - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        Serial.printf("\n current_cluster_1_standby: %d.%04d : \n", decimal, frac);
        #endif

        }

        //#if VT_LOG_LEVEL > 2
        //Serial.printf("current_cluster_2_active : \n");
        //#endif
        iter = 0;
        for (csvString = (VT_CHAR*)flattened_db->repeating_signature_relative_curr_cluster_2_active;; csvString = NULL)
        {
            token = strtok(csvString, ",");
            if (token == NULL)
            {
                break;
            }
            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_cluster_2_active = atof((VT_CHAR*)token);
            temp_value=cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_cluster_2_active;
            iter++;

        decimal    = temp_value;
        frac_float = temp_value - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        Serial.printf("\n current_cluster_2_active: %d.%04d : \n", decimal, frac);
        #endif

        }
        //#if VT_LOG_LEVEL > 2
        //Serial.printf("current_average : \n");
        //#endif
        iter = 0;
        for (csvString = (VT_CHAR*)flattened_db->repeating_signature_relative_curr_average;; csvString = NULL)
        {
            token = strtok(csvString, ",");
            if (token == NULL)
            {
                break;
            }
            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_average = atof((VT_CHAR*)token);
            temp_value=cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_average;
            iter++;

        decimal    = temp_value;
        frac_float = temp_value - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        Serial.printf("\n current_average: %d.%04d : \n", decimal, frac);
        #endif

        }
        //#if VT_LOG_LEVEL > 2
        //Serial.printf("repeating_signature_duty_cycle : \n");
        //#endif
        iter = 0;
        for (csvString = (VT_CHAR*)flattened_db->repeating_signature_duty_cycle;; csvString = NULL)
        {
            token = strtok(csvString, ",");
            if (token == NULL)
            {
                break;
            }
            cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].duty_cycle = atof((VT_CHAR*)token);
            temp_value=cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].duty_cycle;
            iter++;

        decimal    = temp_value;
        frac_float = temp_value - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        #if VT_LOG_LEVEL > 2
        Serial.printf("\nrepeating_signature_duty_cycle: %d.%04d : \n", decimal, frac);
        #endif

        }

        vt_cs_repeating_signature_type(cs_object);

}