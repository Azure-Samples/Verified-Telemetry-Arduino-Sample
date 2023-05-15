/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
#include "vt_cs_api.h"
#include "vt_cs_calibrate.h"
#include "vt_cs_database.h"
#include "vt_cs_raw_signature_read.h"
#include "vt_cs_sensor_status.h"
#include "vt_debug.h"
#include <math.h>

// VT_VOID vt_currentsense_object_signature_read(VT_CURRENTSENSE_OBJECT* cs_object)
// {
//     //VTLogDebug("Signature read started \r\n");
//     VT_FLOAT sampling_frequencies[VT_CS_MAX_SIGNATURES];
//     VT_UINT num_sampling_frqeuencies = 0;
//     //sampling_frequencies[0]=5000;

//     if (cs_object->mode == VT_MODE_RUNTIME_EVALUATE)
//     // {
//     //    cs_fetch_template_repeating_signature_sampling_frequencies(
//     //         cs_object, sampling_frequencies, VT_CS_MAX_SIGNATURES, &num_sampling_frqeuencies);
//     // }
//     // else
//      {//Serial.printf("IM IN");
//          cs_calibrate_repeating_signatures_compute_sampling_frequencies(
//            cs_object, sampling_frequencies, VT_CS_MAX_SIGNATURES, &num_sampling_frqeuencies);
//      }
//      //num_sampling_frqeuencies = 1;
//     cs_raw_signature_read(cs_object, sampling_frequencies, num_sampling_frqeuencies, VT_CS_SAMPLE_LENGTH);

// }

// VT_VOID vt_currentsense_object_signature_process(VT_CURRENTSENSE_OBJECT* cs_object)
// {
//     //VTLogDebug("Signature processing started \r\n");
//     cs_object->raw_signatures_reader->non_repeating_raw_signature_stop_collection = true;
//     while (cs_object->raw_signatures_reader->repeating_raw_signature_ongoing_collection)
//     {
//         //Serial.printf("in while \n");
//         // do nothing, wait till repeating raw signatures are collected
//     }
//             VT_INT decimal;
//     VT_FLOAT frac_float;
//     VT_INT frac;
//     //Serial.printf("printing rwa sig\n");
//    for (VT_INT iter = 0; iter < VT_CS_SAMPLE_LENGTH; iter++)
//     {
//         decimal    = cs_object->raw_signatures_reader->repeating_raw_signatures[0].current_measured[iter];
//         frac_float = cs_object->raw_signatures_reader->repeating_raw_signatures[0].current_measured[iter] - (VT_FLOAT)decimal;
//         frac       = fabsf(frac_float) * 10000;
//         VTLogDebugNoTag("%d.%04d, ", decimal, frac);
//     }
// }


VT_VOID vt_currentsense_object_signature_read(VT_CURRENTSENSE_OBJECT* cs_object,UINT mode)
{
    VTLogDebug("Signature read started \r\n");
    VT_FLOAT sampling_frequencies[VT_CS_MAX_SIGNATURES];
    VT_UINT num_sampling_frqeuencies = 0;

    //Serial.println("read start");

    if (cs_object->mode == VT_MODE_RUNTIME_EVALUATE)
    {
        cs_fetch_template_repeating_signature_sampling_frequencies(
            cs_object, sampling_frequencies, VT_CS_MAX_SIGNATURES, &num_sampling_frqeuencies);
    }
    else
    {
        cs_calibrate_repeating_signatures_compute_sampling_frequencies(
            cs_object, sampling_frequencies, VT_CS_MAX_SIGNATURES, &num_sampling_frqeuencies);
    }

#if VT_LOG_LEVEL > 2

    Serial.printf("num_sampling_frqeuencies : %d \n",num_sampling_frqeuencies);
    Serial.printf("index 0 : %f ",sampling_frequencies[0]);
    Serial.printf("index 1 : %f ",sampling_frequencies[1]);
    Serial.printf("index 2 : %f ",sampling_frequencies[2]);
    Serial.printf("index 3 : %f ",sampling_frequencies[3]);
    Serial.printf("index 4 : %f ",sampling_frequencies[4]);

    Serial.println("config done");

#endif

    cs_raw_signature_read(cs_object, sampling_frequencies, num_sampling_frqeuencies, VT_CS_SAMPLE_LENGTH,mode);
}

VT_VOID vt_currentsense_object_signature_process(VT_CURRENTSENSE_OBJECT* cs_object)
{
    VTLogDebug("Signature processing started \r\n");
    cs_object->raw_signatures_reader->non_repeating_raw_signature_stop_collection = true;
    while (cs_object->raw_signatures_reader->repeating_raw_signature_ongoing_collection)
    {
        // Serial.printf("in while \n");
        // do nothing, wait till repeating raw signatures are collected
    }

    //             VT_INT decimal;
    // VT_FLOAT frac_float;
    // VT_INT frac;
    // Serial.printf("printing rwa sig\n");
    //      VT_INT decimal;
    //  VT_FLOAT frac_float;
    //  VT_INT frac;

    //     for (VT_INT iter = 0; iter < VT_CS_SAMPLE_LENGTH; iter++)
    // {
    //     decimal    = cs_object->raw_signatures_reader->non_repeating_raw_signature.current_measured[iter];
    //     frac_float = cs_object->raw_signatures_reader->non_repeating_raw_signature.current_measured[iter] - (VT_FLOAT)decimal;
    //     frac       = fabsf(frac_float) * 10000;
    //     Serial.printf("%d.%04d, ", decimal, frac);
    // }
    // Serial.printf("\n");

    switch (cs_object->mode)
    {
        case VT_MODE_RUNTIME_EVALUATE:
            VTLogDebug("Computing Sensor Status \r\n");
            cs_sensor_status(cs_object);
            break;

        case VT_MODE_CALIBRATE:
            VTLogDebug("Calibrating Sensor Fingerprint \r\n");
            cs_calibrate_sensor(cs_object);
            
            #if VT_LOG_LEVEL > 1
            Serial.printf(" Calibration_Done_Count : %d MultiCalibration_Count : %d \n",cs_object->Calibration_Done_Count,MULTICALIBRATION_COUNT);
            //Serial.printf("\n NUM SIG = %d \n", cs_object->fingerprintdb.template_struct.repeating_signatures.num_signatures);
            //Serial.printf(" final T type : %d", cs_object->fingerprintdb.template_type);
            #endif
            
            break;

        case VT_MODE_RECALIBRATE:
            VTLogDebug("Recalibrating Sensor Fingerprint \r\n");
            cs_recalibrate_sensor(cs_object);
            cs_object->mode = VT_MODE_RUNTIME_EVALUATE;
            break;
    }
}