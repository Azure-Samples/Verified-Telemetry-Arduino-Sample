/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/** @file */

#ifndef FREERTOS_VERIFIED_TELEMETRY_H
#define FREERTOS_VERIFIED_TELEMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Azure Provisioning/IoT Hub library includes */
#include <vt_defs.h>
#include <vt_cs_api.h>
#include <vt_fc_api.h>

#define VT_SIGNATURE_TYPE_FALLCURVE    0x01
#define VT_SIGNATURE_TYPE_CURRENTSENSE 0x02

#define VT_MINIMUM_BUFFER_SIZE_BYTES sizeof(VT_CURRENTSENSE_RAW_SIGNATURES_READER)

typedef struct FreeRTOS_VT_CURRENTSENSE_COMPONENT_TAG
{
    /* Name of this component */
    UCHAR component_name_ptr[30];

    /* Component Name Length */
    UINT component_name_length;

    /* Telemetry associated with the currentsense signature component*/
    UCHAR associated_telemetry[100];

    /* Currentsense Object */
    VT_CURRENTSENSE_OBJECT cs_object;

    /* Status of reported properties sent  */
    UINT property_sent;

} FreeRTOS_VT_CURRENTSENSE_COMPONENT;

typedef struct FreeRTOS_VT_FALLCURVE_COMPONENT_TAG
{
    /* Name of this component */
    UCHAR component_name_ptr[VT_COMPONENT_NAME_MAX_LENGTH];

    /* Component Name Length */
    UINT component_name_length;

    /* Telemetry associated with the fallcurve signature component*/
    UCHAR associated_telemetry[VT_ASSOCIATED_TELEMETRY_CSV_MAX_LENGTH];

    /* Status of the telemetry associated with the fallcurve signature component*/
    bool telemetry_status;

    /* Fallcurve Object */
    VT_FALLCURVE_OBJECT fc_object;

    /* Status of reported properties sent  */
    UINT property_sent;

    /* Stores on the scale of 0-100 how much Fingerprint Template can be trusted */
    UINT template_confidence_metric;

    /* Compute sensor status when a global command is issued */
    bool telemetry_status_auto_update;

} FreeRTOS_VT_FALLCURVE_COMPONENT;

union FreeRTOS_VT_SIGNATURE_COMPONENT_UNION_TAG {
    /* FallCurve Component */
    FreeRTOS_VT_FALLCURVE_COMPONENT fc;
    FreeRTOS_VT_CURRENTSENSE_COMPONENT cs;
};

typedef struct FreeRTOS_VT_OBJECT_TAG
{
    /* Signature Object Union */
    union FreeRTOS_VT_SIGNATURE_COMPONENT_UNION_TAG component;

    /* Signature Type */
    UINT signature_type;

    /* Pointer to next component */
    void* next_component;

} FreeRTOS_VT_OBJECT;

typedef struct FreeRTOS_VERIFIED_TELEMETRY_DB_TAG
{
    /* Name of this component */
    UCHAR component_name_ptr[VT_COMPONENT_NAME_MAX_LENGTH];

    UINT component_name_length;

    /* Pointer to first component */
    void* first_component;

    /* Pointer to last component */
    void* last_component;

    /* Number of component*/
    UINT components_num;

    /* Device specific implementations*/
    VT_DEVICE_DRIVER* device_driver;

    /* Enable Verified Telemetry*/
    bool enable_verified_telemetry;

    /* Device Status*/
    bool device_status;

    /* Device Status Property Sent*/
    bool device_status_property_sent;

    /* Pointer to byte buffer passed from application layer, used for fingerprint calculation/storage */
    CHAR* scratch_buffer;

    /* Length of byte buffer passed from application layer, used for fingerprint calculation/storage */
    UINT scratch_buffer_length;

} FreeRTOS_VERIFIED_TELEMETRY_DB;


/**
 * @brief Initializes Global Verified Telemetry using platform specific device drivers
 *
 * @param[in] verified_telemetry_DB Pointer to variable of type VERIFIED_TELEMETRY_DB storing Verified Telemetry data.
 * @param[in] component_name_ptr Name of the PnP component. Example - "vTDevice"
 * @param[in] enable_verified_telemetry User specified value to set Verified Telemetry active or inactive, can also be configured
 * during runtime from a writable Digital Twin property.
 * @param[in] device_driver The platform specific device driver components for interacting with the device hardware.
 *
 * @retval NX_AZURE_IOT_SUCCESS upon success or an error code upon failure.
 */

int FreeRTOS_vt_init(FreeRTOS_VERIFIED_TELEMETRY_DB* verified_telemetry_DB,
    UCHAR* component_name_ptr,
    bool enable_verified_telemetry,
    VT_DEVICE_DRIVER* device_driver,
    CHAR* scratch_buffer,
    UINT scratch_buffer_length);

/**
 * @brief Initializes Verified Telemetry for a particular sensor data stream
 *
 * @param[in] verified_telemetry_DB Pointer to variable of type VERIFIED_TELEMETRY_DB storing Verified Telemetry data.
 * @param[in] handle Pointer to variable of type NX_VT_OBJECT storing collection settings and configuration data for a particular
 * sensor telemetry.
 * @param[in] component_name_ptr Name of the sensor.  Example - "accelerometer" This would be prepended with 'vT' by VT library
 * @param[in] signature_type One of the defined signature types. Currently available types - VT_SIGNATURE_TYPE_FALLCURVE
 * @param[in] associated_telemetry Telmetries associated with this sensor, separated by commas  Example - "accelerometerX,
 * accelerometerY, accelerometerZ"
 * @param[in] telemetry_status_auto_update User specified value to control whether fingerprint computation for the sensor should
 * be invoked when nx_vt_compute_evaluate_fingerprint_all_sensors is called
 * @param[in] sensor_handle The sensor specific connection configuration for collecting VT signatures.
 *
 * @retval NX_AZURE_IOT_SUCCESS upon success or an error code upon failure.
 */

int FreeRTOS_vt_signature_init(FreeRTOS_VERIFIED_TELEMETRY_DB* verified_telemetry_DB,
    FreeRTOS_VT_OBJECT* handle,
    UCHAR* component_name_ptr,
    UINT signature_type,
    UCHAR* associated_telemetry,
    bool telemetry_status_auto_update,
    VT_SENSOR_HANDLE* sensor_handle);

int FreeRTOS_vt_currentsense_signature_read(FreeRTOS_VT_CURRENTSENSE_COMPONENT* handle,
    UCHAR* associated_telemetry,
    UINT associated_telemetry_length,
    bool toggle_verified_telemetry,UINT mode);

int FreeRTOS_vt_currentsense_signature_process(FreeRTOS_VT_CURRENTSENSE_COMPONENT* handle,
    UCHAR* associated_telemetry,
    UINT associated_telemetry_length,
    bool toggle_verified_telemetry);


int FreeRTOS_vt_signature_read(
    FreeRTOS_VERIFIED_TELEMETRY_DB* verified_telemetry_DB, UCHAR* associated_telemetry, UINT associated_telemetry_length,UINT mode);

UINT FreeRTOS_vt_signature_process(
    FreeRTOS_VERIFIED_TELEMETRY_DB* verified_telemetry_DB, UCHAR* associated_telemetry, UINT associated_telemetry_length);


#ifdef __cplusplus
}
#endif

#endif /* FREERTOS_VERIFIED_TELEMETRY_H */
