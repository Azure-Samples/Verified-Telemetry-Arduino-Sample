#include<arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include<math.h>
#include "sample_vt_device_driver.h"
#include <MCP3XXX.h>
#include <sps30.h>


MCP3XXX_<10, 4, 312500> adc;

unsigned long current_time=0;
unsigned long previous_time=0;
unsigned long cycle_time=0;

/* Variables needed for External ADC Buffer Read */

/* Sensor Hardware declaration */

//sensor_3
uint16_t vt_adc_id_sensor_3 = SAMPLE_EXTERNAL_ADC_TYPE_ID;
uint16_t vt_adc_controller_sensor_3 = 0;
uint32_t vt_adc_channel_sensor_3 = 0;

//sensor_4
uint16_t vt_adc_id_sensor_4 = SAMPLE_EXTERNAL_ADC_TYPE_ID;
uint16_t vt_adc_controller_sensor_4 = 0;
uint32_t vt_adc_channel_sensor_4 = 1;



float* adc_mcp3204_read_buffer_local;
uint16_t adc_mcp3204_read_buffer_length_local;
uint16_t adc_mcp3204_read_buffer_datapoints_stored = 0;
typedef void (*VT_ADC_BUFFER_READ_CALLBACK_FUNC)(void);
typedef uint16_t (*VT_ADC_BUFFER_READ_FULL_CALLBACK_FUNC)(uint16_t mode);
VT_ADC_BUFFER_READ_CALLBACK_FUNC adc_mcp3204_read_buffer_half_complete_callback;
VT_ADC_BUFFER_READ_FULL_CALLBACK_FUNC adc_mcp3204_read_buffer_full_complete_callback;


void sps_setup() {
  int16_t ret;
  uint8_t auto_clean_days = 4;
  uint32_t auto_clean;

  delay(2000);

  sensirion_i2c_init();

  ret = sps30_start_measurement();
  if (ret < 0) {
    Serial.print("error starting measurement\n");
  }

  delay(1000);
}

int sps_loop() {
  struct sps30_measurement m;
  char serial[SPS30_MAX_SERIAL_LEN];
  uint16_t data_ready;
  int16_t ret;

    ret = sps30_read_data_ready(&data_ready);
    if (ret < 0) {
      Serial.print("error reading data-ready flag: ");
      Serial.println(ret);
    } else if (!data_ready)
      Serial.print("data not ready, no new measurement available\n");

    delay(100); /* retry in 100ms */

  ret = sps30_read_measurement(&m);

  delay(1000);

  return (int)m.mc_2p5;
}

unsigned long vt_tick()
{
    uint64_t task_counter_value;

    task_counter_value=millis();

    return task_counter_value;
}

void vt_adc_buffer_read(uint16_t adc_id,
    void* adc_controller,
    void* adc_channel,
    float* adc_read_buffer,
    uint16_t buffer_length,
    float desired_sampling_frequency,
    float* set_sampling_frequency,
    void (*vt_adc_buffer_read_conv_half_cplt_callback)(),
    uint16_t (*vt_adc_buffer_read_conv_cplt_callback)(uint16_t mode))
{

  //bool read_complete=false;

  adc.begin(10);
  //Serial.println("buffer_Read_start");

    bool readComplete=false;

    current_time=micros();
    cycle_time=current_time-previous_time;
    previous_time=current_time;
    //Serial.print("cycle_time : ");
    //Serial.println(cycle_time);

    //ESP.wdtFeed();
  if (adc_id == SAMPLE_EXTERNAL_ADC_TYPE_ID)
  {
    
    adc_mcp3204_read_buffer_half_complete_callback = vt_adc_buffer_read_conv_half_cplt_callback;
    adc_mcp3204_read_buffer_full_complete_callback = vt_adc_buffer_read_conv_cplt_callback;
    adc_mcp3204_read_buffer_local                  = adc_read_buffer;
    adc_mcp3204_read_buffer_length_local           = buffer_length;
    adc_mcp3204_read_buffer_datapoints_stored      = 0;
    *set_sampling_frequency=12970;
  while(adc_mcp3204_read_buffer_datapoints_stored <= adc_mcp3204_read_buffer_length_local)
    {
      wdt_reset();
      //.println(adc_mcp3204_read_buffer_datapoints_stored);
      adc_mcp3204_read_buffer_local[adc_mcp3204_read_buffer_datapoints_stored] = adc.analogRead(*((uint8_t*)adc_channel));
      //adc_mcp3204_read_buffer_local[adc_mcp3204_read_buffer_datapoints_stored] = random(10,20);
      adc_mcp3204_read_buffer_datapoints_stored++;

      if (adc_mcp3204_read_buffer_datapoints_stored == adc_mcp3204_read_buffer_length_local / 2)
      {  //Serial.println("half_complete");
          adc_mcp3204_read_buffer_half_complete_callback();
      }
      else if (adc_mcp3204_read_buffer_datapoints_stored == adc_mcp3204_read_buffer_length_local)
      {  //Serial.println("full_complete");
          readComplete=adc_mcp3204_read_buffer_full_complete_callback(1);
          if(readComplete==false)
          {
            adc_mcp3204_read_buffer_datapoints_stored=0;
          }
          else 
          {break;}

          //break;
          //Serial.println("read finish");
      }
    }
    
  }
  //Serial.println("buffer_Read_stop");

}
  
