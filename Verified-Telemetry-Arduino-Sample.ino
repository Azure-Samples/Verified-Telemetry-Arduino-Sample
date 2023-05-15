#include <vt_cs_api.h>
#include <vt_debug.h>

#include "sample_freertos_verified_telemetry_init.h"
#include "sample_vt_device_driver.h"
#include "verified_telemetry_middleware.h"
#include <EEPROM.h>

//Using a DB pointer 
FreeRTOS_VERIFIED_TELEMETRY_DB* verified_telemetry_DB;
//Name strings according the name set in sample_freertos_verified_telemetry_init.cpp
char telemetry_name_pmsExternal1Raw[]  = "PMSExternal1";
char telemetry_name_pmsExternal2Raw[] = "SampleSecondSensor";

//Name with VT tag
#define SPS30_VT_NAME "vTPMSExternal1"
#define SAMPLE_SECOND_SENSR_VT_NAME "vTSampleSecondSensor"
#define VT_EEPROM_OFFSET 128

//structure to hold sensor status
struct VtSensorStatus
{
bool SPS30_status;
bool SampleSecondSensor_status;
}VtSensorStatusStruct;

//various DB and calibration variables
int eepromIndex;
FreeRTOS_VT_CURRENTSENSE_COMPONENT* handle;
VT_CURRENTSENSE_DATABASE_FLATTENED flattened_db_local;
VT_CURRENTSENSE_DATABASE_FLATTENED * flattened_db_local_ptr;
uint8_t firstBoot;
bool db_update_needed=false;
uint16_t confidence_metric_local=0;
int initialCalibDone=0;
volatile int sensorCalibrateNum = 0; 

void VT_setup() {

  sps_setup();

  ESP.wdtEnable(0);
  Serial.begin(115200);
  EEPROM.begin(1280);
  eepromIndex=-1;
  verified_telemetry_DB = sample_nx_verified_telemetry_user_init();


  UINT iter               = 0;
  UINT components_num     = verified_telemetry_DB->components_num;
  void* component_pointer = verified_telemetry_DB->first_component;
  
  EEPROM.get(VT_EEPROM_OFFSET + components_num*sizeof(VT_CURRENTSENSE_DATABASE_FLATTENED)+1, firstBoot);

  
  if(firstBoot!=128)
  { 
    Serial.println("Calibrating for first boot");  
    VT_Command_process("resetDigitalSensorOne");
    VT_Command_process("resetDigitalSensorTwo");
    EEPROM.put(VT_EEPROM_OFFSET + components_num*sizeof(VT_CURRENTSENSE_DATABASE_FLATTENED)+1, 128);    
    EEPROM.commit();
  }
  else 
  {
    for (iter = 0; iter < components_num; iter++)
    {   
      if (((FreeRTOS_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
      {
        FreeRTOS_VT_CURRENTSENSE_COMPONENT* handle=&(((FreeRTOS_VT_OBJECT*)component_pointer)->component.cs);
        EEPROM.get(VT_EEPROM_OFFSET + iter*sizeof(VT_CURRENTSENSE_DATABASE_FLATTENED), flattened_db_local);
        vt_currentsense_object_database_sync(&(handle->cs_object), &flattened_db_local);
      }
      component_pointer = (((FreeRTOS_VT_OBJECT*)component_pointer)->next_component);
    }
      Serial.println("Calibration data fetched from memory.");
  }

}

int VT_Command_process(String uart_command)
{

    char command_sensor_name[64];
    strcpy(command_sensor_name,"vT");

    if(uart_command.compareTo("resetDigitalSensorOne")==0)
    {
      eepromIndex=0;
      Serial.println("Sensor One Reset Command");
      strcat(command_sensor_name,telemetry_name_pmsExternal1Raw);
    }
    else if(uart_command.compareTo("resetDigitalSensorTwo")==0)
    {
      eepromIndex=1;
      Serial.println("Sensor Two Reset Command");
      strcat(command_sensor_name,telemetry_name_pmsExternal2Raw);
    }
    else 
    {
        Serial.println("command not found ");
    }

    unsigned int command_sensor_name_length=0;
    command_sensor_name_length=strlen(command_sensor_name);

    UINT iter               = 0;
    UINT components_num     = verified_telemetry_DB->components_num;
    void* component_pointer = verified_telemetry_DB->first_component;

    for (iter = 0; iter < components_num; iter++)
    {
      if (((FreeRTOS_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
      {

        FreeRTOS_VT_CURRENTSENSE_COMPONENT* handle=&(((FreeRTOS_VT_OBJECT*)component_pointer)->component.cs);
        
        if (strncmp((CHAR*)handle->component_name_ptr, command_sensor_name, handle->component_name_length) == 0)
        {   
            Serial.printf("calibrate for : %.*s \n",handle->component_name_length,(CHAR*)handle->component_name_ptr);
            vt_currentsense_object_sensor_calibrate(&(handle->cs_object));

        }
      }
      component_pointer = (((FreeRTOS_VT_OBJECT*)component_pointer)->next_component);
    }

    //Serial.println(" ");
    return VT_SUCCESS;
}

void VT_loop() {

  int pm2_5 ;

  if (Serial.available() > 0) {
    String string_buffer;
    string_buffer=Serial.readString();
    VT_Command_process(string_buffer);
  }

    // FOR SENSOR 1
    FreeRTOS_vt_signature_read(verified_telemetry_DB,
        (UCHAR*)telemetry_name_pmsExternal1Raw,
        sizeof(telemetry_name_pmsExternal1Raw) - 1,1);

    /* SPS30 READ FUNCTION*/
    pm2_5 = sps_loop();

    FreeRTOS_vt_signature_process(verified_telemetry_DB,
        (UCHAR*)telemetry_name_pmsExternal1Raw,
        sizeof(telemetry_name_pmsExternal1Raw) - 1);


    // FOR SENSOR 2
    FreeRTOS_vt_signature_read(verified_telemetry_DB,
        (UCHAR*)telemetry_name_pmsExternal2Raw,
        sizeof(telemetry_name_pmsExternal2Raw) - 1,1);

    // Sample Second Sensor read function

    FreeRTOS_vt_signature_process(verified_telemetry_DB,
        (UCHAR*)telemetry_name_pmsExternal2Raw,
        sizeof(telemetry_name_pmsExternal2Raw) - 1);


    UINT iter               = 0;
    UINT components_num     = verified_telemetry_DB->components_num;
    void* component_pointer = verified_telemetry_DB->first_component;

    for (iter = 0; iter < components_num; iter++)
    {
      if (((FreeRTOS_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
      {
        FreeRTOS_VT_CURRENTSENSE_COMPONENT* handle=&(((FreeRTOS_VT_OBJECT*)component_pointer)->component.cs);
        
        if (strncmp((CHAR*)handle->component_name_ptr, SPS30_VT_NAME, handle->component_name_length) == 0)
        {
        VtSensorStatusStruct.SPS30_status=((handle->cs_object.sensor_status==0)?true:false);
        //Serial.printf("{'SPS_status':%d",VtSensorStatusStruct.SPS30_status);
        }
        else if (strncmp((CHAR*)handle->component_name_ptr, SAMPLE_SECOND_SENSR_VT_NAME, handle->component_name_length) == 0)
        {
        VtSensorStatusStruct.SampleSecondSensor_status=((handle->cs_object.sensor_status==0)?true:false);
        //Serial.printf("Second_Sensor_Status : %d \n",VtSensorStatusStruct.SampleSecondSensor_status);
        
        }       
      }
      component_pointer = (((FreeRTOS_VT_OBJECT*)component_pointer)->next_component);
    }

    Serial.printf("{'SPS_status': %d",VtSensorStatusStruct.SPS30_status);
    Serial.printf(",'SPS_Data': %d} \n",pm2_5);
    Serial.printf("{'Second_Sensor_Status': %d ",VtSensorStatusStruct.SampleSecondSensor_status);
    Serial.printf(",'Dummy_Second_Sensor_Data': %d} \n",random(1, 10));
    
    if(eepromIndex>=0)
    {

      UINT iter               = 0;
      UINT components_num     = eepromIndex;
      void* component_pointer = verified_telemetry_DB->first_component;

      for (iter = 0; iter <= components_num; iter++)
      {
        if (((FreeRTOS_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
        {

          handle=&(((FreeRTOS_VT_OBJECT*)component_pointer)->component.cs);
        }

        if(handle->cs_object.Calibration_Done_Count==MULTICALIBRATION_COUNT)
        {
          //Serial.printf(" \n DB SAVED FOR: %.*s \n",handle->component_name_length,(CHAR*)handle->component_name_ptr);
          vt_currentsense_object_database_fetch(&(handle->cs_object),&flattened_db_local,&db_update_needed,&confidence_metric_local);

          if(db_update_needed==true)
            {
              Serial.printf(" \n DB SAVED FOR: %.*s \n",handle->component_name_length,(CHAR*)handle->component_name_ptr);
              EEPROM.put(VT_EEPROM_OFFSET + iter*sizeof(VT_CURRENTSENSE_DATABASE_FLATTENED), flattened_db_local);
              //Serial.println(iter);              
              eepromIndex=-1;
              EEPROM.commit();
            }
            
          handle->cs_object.Calibration_Done_Count++;
        }

        component_pointer = (((FreeRTOS_VT_OBJECT*)component_pointer)->next_component);
      }

    }

}

void setup()
{
VT_setup();
}

void loop()
{

VT_loop();
delay(5000);

}
