
#include<arduino.h>
/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
#include <vt_cs_api.h>
#include <vt_fc_api.h>
#include <vt_cs_database.h>
#include "sample_freertos_verified_telemetry_init.h"
#include "sample_vt_device_driver.h"
#include "verified_telemetry_middleware.h"
#include <vt_defs.h>
#include <string.h>

int FreeRTOS_vt_init(FreeRTOS_VERIFIED_TELEMETRY_DB* verified_telemetry_DB,
    UCHAR* component_name_ptr,
    bool enable_verified_telemetry,
    VT_DEVICE_DRIVER* device_driver,
    CHAR* scratch_buffer,
    UINT scratch_buffer_length)
{
    strncpy((CHAR*)verified_telemetry_DB->component_name_ptr,
        (CHAR*)component_name_ptr,
        sizeof(verified_telemetry_DB->component_name_ptr));
    verified_telemetry_DB->component_name_length       = strlen((const char*)component_name_ptr);
    verified_telemetry_DB->enable_verified_telemetry   = enable_verified_telemetry;
    verified_telemetry_DB->device_status_property_sent = false;
    verified_telemetry_DB->components_num              = 0;
    verified_telemetry_DB->first_component             = NULL;
    verified_telemetry_DB->last_component              = NULL;

    verified_telemetry_DB->device_driver = device_driver;
    verified_telemetry_DB->scratch_buffer        = scratch_buffer;
    verified_telemetry_DB->scratch_buffer_length = scratch_buffer_length;

    return VT_SUCCESS;
}

int FreeRTOS_vt_fallcurve_init(FreeRTOS_VT_FALLCURVE_COMPONENT* handle,
    UCHAR* component_name_ptr,
    VT_DEVICE_DRIVER* device_driver,
    VT_SENSOR_HANDLE* sensor_handle,
    UCHAR* associated_telemetry,
    bool telemetry_status_auto_update)
{
    CHAR vt_component_name[VT_COMPONENT_NAME_MAX_LENGTH];
    VT_INT str_manipulation_return;
    VT_INT str_buffer_space_available;

    if (handle == NULL)
    {
        return VT_ERROR;
    }

    memset(vt_component_name, 0, sizeof(vt_component_name));
    str_manipulation_return = snprintf(vt_component_name, sizeof(vt_component_name), "vT");
    if (str_manipulation_return < 0 || (VT_UINT)str_manipulation_return > sizeof(vt_component_name))
    {
        Serial.print("Flattened Database Buffer Overflow! \r\n");
    }
    str_buffer_space_available = sizeof(vt_component_name) - strlen(vt_component_name);
    strncat(vt_component_name, (CHAR*)component_name_ptr, str_buffer_space_available);
    strncpy((CHAR*)handle->component_name_ptr, vt_component_name, sizeof(handle->component_name_ptr) - 1);
    handle->component_name_length = strlen(vt_component_name);
    strncpy((CHAR*)handle->associated_telemetry, (CHAR*)associated_telemetry, sizeof(handle->associated_telemetry) - 1);
    handle->telemetry_status             = false;
    handle->property_sent                = 0;
    handle->template_confidence_metric   = 0;
    handle->telemetry_status_auto_update = telemetry_status_auto_update;

    vt_fallcurve_object_initialize(&(handle->fc_object), device_driver, sensor_handle);

    return VT_SUCCESS;
}

int FreeRTOS_vt_currentsense_init(FreeRTOS_VT_CURRENTSENSE_COMPONENT* handle,
    UCHAR* component_name_ptr,
    VT_DEVICE_DRIVER* device_driver,
    VT_SENSOR_HANDLE* sensor_handle,
    UCHAR* associated_telemetry,
    CHAR* shared_buffer,
    UINT shared_buffer_size)
{
    int xResult;
    CHAR vt_component_name[VT_COMPONENT_NAME_MAX_LENGTH];
    VT_INT str_manipulation_return;
    VT_INT str_buffer_space_available;

    if (handle == NULL)
    {
        return VT_ERROR;
    }

    memset(vt_component_name, 0, sizeof(vt_component_name));
    str_manipulation_return = snprintf(vt_component_name, sizeof(vt_component_name), "vT");
    if (str_manipulation_return < 0 || (VT_UINT)str_manipulation_return > sizeof(vt_component_name))
    {
        Serial.print("Flattened Database Buffer Overflow! \r\n");
    }
    str_buffer_space_available = sizeof(vt_component_name) - strlen(vt_component_name);
    strncat(vt_component_name, (CHAR*)component_name_ptr, str_buffer_space_available);
    strncpy((CHAR*)handle->component_name_ptr, vt_component_name, sizeof(handle->component_name_ptr) - 1);
    handle->component_name_length = strlen(vt_component_name);
    strncpy((CHAR*)handle->associated_telemetry, (CHAR*)associated_telemetry, sizeof(handle->associated_telemetry) - 1);
    
    handle->property_sent                = 0;
    
    xResult= vt_currentsense_object_initialize(&(handle->cs_object), device_driver, sensor_handle,shared_buffer, shared_buffer_size);

    return xResult;
}

int FreeRTOS_vt_currentsense_signature_read(FreeRTOS_VT_CURRENTSENSE_COMPONENT* handle,
    UCHAR* associated_telemetry,
    UINT associated_telemetry_length,
    bool toggle_verified_telemetry,UINT mode)
{
    if (strlen((CHAR*)handle->associated_telemetry) != associated_telemetry_length ||
        strncmp((CHAR*)handle->associated_telemetry, (CHAR*)associated_telemetry, associated_telemetry_length) != 0)
    {
        return (VT_ERROR);
    }
    if (!toggle_verified_telemetry)
    {
        return (VT_ERROR);
    }
    
    vt_currentsense_object_signature_read(&(handle->cs_object),mode);
    
    return (VT_SUCCESS);
}

int FreeRTOS_vt_currentsense_signature_process(FreeRTOS_VT_CURRENTSENSE_COMPONENT* handle,
    UCHAR* associated_telemetry,
    UINT associated_telemetry_length,
    bool toggle_verified_telemetry)
{
    if (strlen((CHAR*)handle->associated_telemetry) != associated_telemetry_length ||
        strncmp((CHAR*)handle->associated_telemetry, (CHAR*)associated_telemetry, associated_telemetry_length) != 0)
    {
        return (VT_ERROR);
    }
    if (!toggle_verified_telemetry)
    {
        return (VT_ERROR);
    }
    vt_currentsense_object_signature_process(&(handle->cs_object));
    return (VT_SUCCESS);
}

int FreeRTOS_vt_signature_read(
    FreeRTOS_VERIFIED_TELEMETRY_DB* verified_telemetry_DB, UCHAR* associated_telemetry, UINT associated_telemetry_length,UINT mode)
{
    UINT iter                      = 0;
    UINT components_num            = verified_telemetry_DB->components_num;
    void* component_pointer        = verified_telemetry_DB->first_component;
    bool enable_verified_telemetry = verified_telemetry_DB->enable_verified_telemetry;

    if (!enable_verified_telemetry)
    {
        
        return (VT_ERROR);
    }
   
    for (iter = 0; iter < components_num; iter++)
    {
        if (((FreeRTOS_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
        {
            if (FreeRTOS_vt_currentsense_signature_read(&(((FreeRTOS_VT_OBJECT*)component_pointer)->component.cs),
                    associated_telemetry,
                    associated_telemetry_length,
                    enable_verified_telemetry,mode) == VT_SUCCESS)
            {
                
                return VT_SUCCESS;
            }
        }
        component_pointer = (((FreeRTOS_VT_OBJECT*)component_pointer)->next_component);
    }
    
    return VT_ERROR;
}

UINT FreeRTOS_vt_signature_process(
    FreeRTOS_VERIFIED_TELEMETRY_DB* verified_telemetry_DB, UCHAR* associated_telemetry, UINT associated_telemetry_length)
{
    UINT iter                      = 0;
    UINT components_num            = verified_telemetry_DB->components_num;
    void* component_pointer        = verified_telemetry_DB->first_component;
    bool enable_verified_telemetry = verified_telemetry_DB->enable_verified_telemetry;

    if (!enable_verified_telemetry)
    {
        return (VT_ERROR);
    }

    for (iter = 0; iter < components_num; iter++)
    {
        if (((FreeRTOS_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
        {
            if (FreeRTOS_vt_currentsense_signature_process(&(((FreeRTOS_VT_OBJECT*)component_pointer)->component.cs),
                    associated_telemetry,
                    associated_telemetry_length,
                    enable_verified_telemetry) == VT_SUCCESS)
            {
                return VT_SUCCESS;
            }
        }
        component_pointer = (((FreeRTOS_VT_OBJECT*)component_pointer)->next_component);
    }
    return VT_ERROR;
}


int FreeRTOS_vt_signature_init(FreeRTOS_VERIFIED_TELEMETRY_DB* verified_telemetry_DB,
    FreeRTOS_VT_OBJECT* handle,
    UCHAR* component_name_ptr,
    UINT signature_type,
    UCHAR* associated_telemetry,
    bool telemetry_status_auto_update,
    VT_SENSOR_HANDLE* sensor_handle)
{
    if (signature_type != VT_SIGNATURE_TYPE_FALLCURVE && signature_type != VT_SIGNATURE_TYPE_CURRENTSENSE)
    {
        return VT_ERROR;
    }
    verified_telemetry_DB->components_num++;
    if (verified_telemetry_DB->first_component == NULL)
    {
        verified_telemetry_DB->first_component = (void*)handle;
    }
    else
    {
        ((FreeRTOS_VT_OBJECT*)(verified_telemetry_DB->last_component))->next_component = (void*)handle;
    }
    verified_telemetry_DB->last_component = (void*)handle;

    handle->next_component = NULL;
    handle->signature_type = signature_type;
    if (signature_type == VT_SIGNATURE_TYPE_FALLCURVE)
    {
        FreeRTOS_vt_fallcurve_init(&(handle->component.fc),
            component_name_ptr,
            verified_telemetry_DB->device_driver,
            sensor_handle,
            associated_telemetry,
            telemetry_status_auto_update);
    }
    else if (signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
    {
        FreeRTOS_vt_currentsense_init(&(handle->component.cs),
            component_name_ptr,
            verified_telemetry_DB->device_driver,
            sensor_handle,
            associated_telemetry,
            verified_telemetry_DB->scratch_buffer,
            verified_telemetry_DB->scratch_buffer_length);
    }
    return VT_SUCCESS;
}
