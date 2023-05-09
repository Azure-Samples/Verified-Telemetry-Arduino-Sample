/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
#include "vt_cs_api.h"
#include "vt_cs_database.h"
#include <string.h>

static VT_CHAR string_buffer[128];


VT_VOID vt_currentsense_object_database_fetch(VT_CURRENTSENSE_OBJECT* cs_object,
    VT_CURRENTSENSE_DATABASE_FLATTENED* flattened_db,
    VT_BOOL* db_updated,
    VT_UINT* template_confidence_metric)
{
    static VT_CHAR string_buffer[40];
    static VT_CHAR string_element[15];
    VT_INT decimal;
    VT_FLOAT frac_float;
    VT_INT frac;

    memset(string_buffer, 0, sizeof(string_buffer));
    snprintf(string_buffer, sizeof(string_buffer), "%03d", cs_object->fingerprintdb.template_type);
    strcpy((VT_CHAR*)flattened_db->template_type, string_buffer);
    
        decimal    = cs_object->fingerprintdb.template_struct.non_repeating_signature.avg_curr_off;
        frac_float = cs_object->fingerprintdb.template_struct.non_repeating_signature.avg_curr_off - (VT_FLOAT)decimal;
        frac       = frac_float * 10000;
        memset(string_buffer, 0, sizeof(string_buffer));
        snprintf(string_buffer, sizeof(string_buffer), "%d.%04d", decimal, frac);
        strcpy((VT_CHAR*)flattened_db->non_repeating_signature_avg_curr_off, string_buffer);

        decimal    = cs_object->fingerprintdb.template_struct.non_repeating_signature.avg_curr_on;
        frac_float = cs_object->fingerprintdb.template_struct.non_repeating_signature.avg_curr_on - (VT_FLOAT)decimal;
        frac       = frac_float * 10000;
        memset(string_buffer, 0, sizeof(string_buffer));
        snprintf(string_buffer, sizeof(string_buffer), "%d.%04d", decimal, frac);
        strcpy((VT_CHAR*)flattened_db->non_repeating_signature_avg_curr_on, string_buffer);
        
        memset(string_buffer, 0, sizeof(string_buffer));
        snprintf(
            string_buffer, sizeof(string_buffer), "%03d", cs_object->fingerprintdb.template_struct.repeating_signatures.num_signatures);
        strcpy((VT_CHAR*)flattened_db->repeating_signature_num_signatures, string_buffer);

        decimal    = cs_object->fingerprintdb.template_struct.repeating_signatures.offset_current;
        frac_float = cs_object->fingerprintdb.template_struct.repeating_signatures.offset_current - (VT_FLOAT)decimal;
        frac       = frac_float * 10000;
        memset(string_buffer, 0, sizeof(string_buffer));
        snprintf(string_buffer, sizeof(string_buffer), "%d.%04d", decimal, frac);
        strcpy((VT_CHAR*)flattened_db->repeating_signature_offset_curr, string_buffer);

        decimal    = cs_object->fingerprintdb.template_struct.repeating_signatures.lowest_sample_freq;
        frac_float = cs_object->fingerprintdb.template_struct.repeating_signatures.lowest_sample_freq - (VT_FLOAT)decimal;
        frac       = frac_float * 10000;
        memset(string_buffer, 0, sizeof(string_buffer));
        snprintf(string_buffer, sizeof(string_buffer), "%d.%04d", decimal, frac);
        strcpy((VT_CHAR*)flattened_db->repeating_signature_lowest_sample_freq, string_buffer);
        
        memset(string_buffer, 0, sizeof(string_buffer));
        for (VT_UINT iter = 0; iter < 5; iter++)
        {
            decimal = cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].sampling_freq;
            frac_float =
                cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].sampling_freq - (VT_FLOAT)decimal;
            frac = frac_float * 10000;
            memset(string_element, 0, sizeof(string_element));
            snprintf(string_element, sizeof(string_element), "%d.%04d", decimal, frac);
            if (iter > 0)
            {
                strcat(string_buffer, ",");
            }
            strcat(string_buffer, string_element);
        }
        strcpy((VT_CHAR*)flattened_db->repeating_signature_sampling_freq, string_buffer);
        

        memset(string_buffer, 0, sizeof(string_buffer));
        for (VT_UINT iter = 0; iter < 5; iter++)
        {
            decimal = cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq;
            frac_float =
                cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].signature_freq - (VT_FLOAT)decimal;
            frac = frac_float * 10000;
            memset(string_element, 0, sizeof(string_element));
            snprintf(string_element, sizeof(string_element), "%d.%04d", decimal, frac);
            if (iter > 0)
            {
                strcat(string_buffer, ",");
            }
            strcat(string_buffer, string_element);
        }
        strcpy((VT_CHAR*)flattened_db->repeating_signature_freq, string_buffer);

        memset(string_buffer, 0, sizeof(string_buffer));
        for (VT_UINT iter = 0; iter < 5; iter++)
        {
            decimal = cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].sec_signature_freq;
            frac_float =
                cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].sec_signature_freq - (VT_FLOAT)decimal;
            frac = frac_float * 10000;
            memset(string_element, 0, sizeof(string_element));
            snprintf(string_element, sizeof(string_element), "%d.%04d", decimal, frac);
            if (iter > 0)
            {
                strcat(string_buffer, ",");
            }
            strcat(string_buffer, string_element);
        }
        strcpy((VT_CHAR*)flattened_db->repeating_sec_signature_freq, string_buffer);

        memset(string_buffer, 0, sizeof(string_buffer));
        for (VT_UINT iter = 0; iter < 5; iter++)
        {
            decimal = cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].relative_curr_draw;
            frac_float =
                cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].relative_curr_draw - (VT_FLOAT)decimal;
            frac = frac_float * 10000;
            memset(string_element, 0, sizeof(string_element));
            snprintf(string_element, sizeof(string_element), "%d.%04d", decimal, frac);
            if (iter > 0)
            {
                strcat(string_buffer, ",");
            }
            strcat(string_buffer, string_element);
        }
        strcpy((VT_CHAR*)flattened_db->repeating_signature_relative_curr_draw, string_buffer);


        memset(string_buffer, 0, sizeof(string_buffer));
        for (VT_UINT iter = 0; iter < 5; iter++)
        {
            decimal = cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_cluster_1_standby;
            frac_float =
                cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_cluster_1_standby - (VT_FLOAT)decimal;
            frac = frac_float * 10000;
            memset(string_element, 0, sizeof(string_element));
            snprintf(string_element, sizeof(string_element), "%d.%04d", decimal, frac);
            if (iter > 0)
            {
                strcat(string_buffer, ",");
            }
            strcat(string_buffer, string_element);
        }
        strcpy((VT_CHAR*)flattened_db->repeating_signature_relative_curr_cluster_1_standby, string_buffer);


        memset(string_buffer, 0, sizeof(string_buffer));
        for (VT_UINT iter = 0; iter < 5; iter++)
        {
            decimal = cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_cluster_2_active;
            frac_float =
                cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_cluster_2_active - (VT_FLOAT)decimal;
            frac = frac_float * 10000;
            memset(string_element, 0, sizeof(string_element));
            snprintf(string_element, sizeof(string_element), "%d.%04d", decimal, frac);
            if (iter > 0)
            {
                strcat(string_buffer, ",");
            }
            strcat(string_buffer, string_element);
        }
        strcpy((VT_CHAR*)flattened_db->repeating_signature_relative_curr_cluster_2_active, string_buffer);


        memset(string_buffer, 0, sizeof(string_buffer));
        for (VT_UINT iter = 0; iter < 5; iter++)
        {
            decimal = cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_average;
            frac_float =
                cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].current_average - (VT_FLOAT)decimal;
            frac = frac_float * 10000;
            memset(string_element, 0, sizeof(string_element));
            snprintf(string_element, sizeof(string_element), "%d.%04d", decimal, frac);
            if (iter > 0)
            {
                strcat(string_buffer, ",");
            }
            strcat(string_buffer, string_element);
        }
        strcpy((VT_CHAR*)flattened_db->repeating_signature_relative_curr_average, string_buffer);


        memset(string_buffer, 0, sizeof(string_buffer));
        for (VT_UINT iter = 0; iter < 5; iter++)
        {
            decimal    = cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].duty_cycle;
            frac_float = cs_object->fingerprintdb.template_struct.repeating_signatures.signatures[iter].duty_cycle - (VT_FLOAT)decimal;
            frac       = frac_float * 10000;
            memset(string_element, 0, sizeof(string_element));
            snprintf(string_element, sizeof(string_element), "%d.%04d", decimal, frac);
            if (iter > 0)
            {
                strcat(string_buffer, ",");
            }
            strcat(string_buffer, string_element);
        }
        strcpy((VT_CHAR*)flattened_db->repeating_signature_duty_cycle, string_buffer);


    
    *template_confidence_metric = cs_object->template_confidence_metric;

    if(cs_object->Calibration_Done_Count>=MULTICALIBRATION_COUNT)
    {
        cs_object->db_updated = VT_DB_UPDATED;
    }

    if (cs_object->db_updated == VT_DB_NOT_UPDATED)
    {
        *db_updated = false;
    }
    else
    {
        *db_updated           = true;
        cs_object->db_updated = VT_DB_NOT_UPDATED;
    }
}


VT_VOID vt_currentsense_debug_database_fetch(VT_CURRENTSENSE_OBJECT* cs_object,
    char* flattened_debug_db_ptr)
{

    memset(flattened_debug_db_ptr, 0, sizeof(flattened_debug_db_ptr));

    //static VT_CHAR string_element[15];
    VT_INT decimal;
    VT_FLOAT frac_float;
    VT_INT frac;

       //memset(string_buffer, 0, sizeof(string_buffer));
       //snprintf(string_buffer, sizeof(string_buffer), "%d.%04d", decimal, frac);
       //strcpy((VT_CHAR*)flattened_db->non_repeating_signature_avg_curr_off, string_buffer);
    decimal    = cs_object->fingerprintdb.debug_template.non_rep_current_drift;
    frac_float = cs_object->fingerprintdb.debug_template.non_rep_current_drift - (VT_FLOAT)decimal;
    frac       = frac_float * 10000;
    memset(string_buffer, 0, sizeof(string_buffer));
    snprintf(string_buffer, sizeof(string_buffer), "NonRep_Valid_Drift:%d:%d.%04d----",cs_object->fingerprintdb.debug_template.non_rep_drift_valid, decimal, frac);
    strcat(flattened_debug_db_ptr, string_buffer);


    decimal    = cs_object->fingerprintdb.debug_template.offset_current_drift;
    frac_float = cs_object->fingerprintdb.debug_template.offset_current_drift - (VT_FLOAT)decimal;
    frac       = frac_float * 10000;
    memset(string_buffer, 0, sizeof(string_buffer));
    snprintf(string_buffer, sizeof(string_buffer), "Offset_Valid_Drift:%d:%d.%04d----",cs_object->fingerprintdb.debug_template.offset_current_drift_valid, decimal, frac);
    strcat(flattened_debug_db_ptr, string_buffer);

    memset(string_buffer, 0, sizeof(string_buffer));
    snprintf(string_buffer, sizeof(string_buffer), "SamplingFreq_Feature-Valid-Drift_Current-Valid-Drift:");
    strcat(flattened_debug_db_ptr, string_buffer);

    for (VT_UINT iter = 0; iter < cs_object->fingerprintdb.template_struct.repeating_signatures.num_signatures; iter++)
    {
        memset(string_buffer, 0, sizeof(string_buffer));

        decimal    = cs_object->fingerprintdb.debug_template.rep_sampling_freq[iter];
        frac_float = cs_object->fingerprintdb.debug_template.rep_sampling_freq[iter] - (VT_FLOAT)decimal;
        frac       = frac_float * 10000;

        snprintf(string_buffer, sizeof(string_buffer), "%d.%04d:",decimal, frac);
        strcat(flattened_debug_db_ptr,string_buffer );

        memset(string_buffer, 0, sizeof(string_buffer));

        decimal    = cs_object->fingerprintdb.debug_template.rep_feature_vector_drift[iter];
        frac_float = cs_object->fingerprintdb.debug_template.rep_feature_vector_drift[iter] - (VT_FLOAT)decimal;
        frac       = frac_float * 10000;

        snprintf(string_buffer, sizeof(string_buffer), "%d.%04d:", decimal, frac);
        strcat(flattened_debug_db_ptr,string_buffer );

        memset(string_buffer, 0, sizeof(string_buffer));

        decimal    = cs_object->fingerprintdb.debug_template.rep_relative_current_drift[iter];
        frac_float = cs_object->fingerprintdb.debug_template.rep_relative_current_drift[iter] - (VT_FLOAT)decimal;
        frac       = frac_float * 10000;

        snprintf(string_buffer, sizeof(string_buffer), "%d.%04d----", decimal, frac);
        strcat(flattened_debug_db_ptr,string_buffer);

    }

}

VT_VOID vt_currentsense_object_database_message(VT_CURRENTSENSE_OBJECT* cs_object,
    VT_CURRENTSENSE_DATABASE_FLATTENED* flattened_db,
    char* flattened_db_message)
{

    memset(flattened_db_message, 0, sizeof(flattened_db_message));

    strcat(flattened_db_message, "template_type:");
    strcat(flattened_db_message, (char *)(&(flattened_db->template_type)));

    //memset(string_buffer, 0, sizeof(string_buffer));
    //snprintf(string_buffer, sizeof(string_buffer), "template_type : ");
    strcat(flattened_db_message, " template_type:");
    strcat(flattened_db_message, (char *)(&(flattened_db->template_type)));

    strcat(flattened_db_message, " avg_curr_off:");
    strcat(flattened_db_message, (char *)(&(flattened_db->non_repeating_signature_avg_curr_off)));

    strcat(flattened_db_message, " avg_curr_on:");
    strcat(flattened_db_message, (char *)(&(flattened_db->non_repeating_signature_avg_curr_on)));

    strcat(flattened_db_message, " lowest_sample_freq:");
    strcat(flattened_db_message, (char *)(&(flattened_db->repeating_signature_lowest_sample_freq)));

    strcat(flattened_db_message, " sampling_freq:");
    strcat(flattened_db_message, (char *)(&(flattened_db->repeating_signature_sampling_freq)));

    strcat(flattened_db_message, " sec_signature_freq:");
    strcat(flattened_db_message, (char *)(&(flattened_db->repeating_sec_signature_freq)));

    strcat(flattened_db_message, " relative_curr_draw:");
    strcat(flattened_db_message, (char *)(&(flattened_db->repeating_signature_relative_curr_draw)));

    strcat(flattened_db_message, " current_cluster_1_standby:");
    strcat(flattened_db_message, (char *)(&(flattened_db->repeating_signature_relative_curr_cluster_1_standby)));

    strcat(flattened_db_message, " current_cluster_2_active:");
    strcat(flattened_db_message, (char *)(&(flattened_db->repeating_signature_relative_curr_cluster_2_active)));

    strcat(flattened_db_message, " current_average:");
    strcat(flattened_db_message, (char *)(&(flattened_db->repeating_signature_relative_curr_average)));

    strcat(flattened_db_message, " duty_cycle:");
    strcat(flattened_db_message, (char *)(&(flattened_db->repeating_signature_duty_cycle)));

}