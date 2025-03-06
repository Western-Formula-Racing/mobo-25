#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "CAN.h"
#include "state.h"

//digital inputs
#define BSPD_GPIO          GPIO_NUM_37   //BSPD Status Input pin
#define IMD_GPIO           GPIO_NUM_39   //IMD Status Input pin
#define LATCH_GPIO         GPIO_NUM_40   //Latch Status Input pin
#define ORION_GPIO         GPIO_NUM_5    //Orion Status (if orion is used)
#define HV_GPIO            GPIO_NUM_10   //HV Status (AIR+ on)
#define AIRN_GPIO          GPIO_NUM_11   //AIR- Status
#define CANTX              GPIO_NUM_47
#define CANRX              GPIO_NUM_48

#define AMS_LATCH          GPIO_NUM_9    //AMS Latch output pin
#define PRECH_OK           GPIO_NUM_36   //Precharge OK output pin

#define CURSENSE_ADC       GPIO_NUM_2    //Current Sensor Input
#define PRECHSENSE_ADC     GPIO_NUM_1    //HV Precharge Sense Input
#define IMD_PWM            GPIO_NUM_12   //IMD Fault PWM Input

extern "C" void app_main(void)
{

  static const char* TAG = "main";

  vTaskDelay(pdMS_TO_TICKS(100));
  printf("Starting...");

  esp_log_level_set("*",ESP_LOG_INFO);

  // init inputs
  gpio_set_direction(BSPD_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(IMD_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(LATCH_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(ORION_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(HV_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(AIRN_GPIO,GPIO_MODE_INPUT);
  //init outputs
  gpio_set_direction(AMS_LATCH,GPIO_MODE_OUTPUT);
  gpio_set_direction(PRECH_OK,GPIO_MODE_OUTPUT);

  //setup ADC
  adc_oneshot_unit_handle_t adc1_handle;
  adc_oneshot_unit_init_cfg_t adcconfig = {
  .unit_id = ADC_UNIT_1,
  .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
  .ulp_mode = ADC_ULP_MODE_DISABLE,
  };

  ESP_ERROR_CHECK(adc_oneshot_new_unit(&adcconfig, &adc1_handle));

  adc_oneshot_chan_cfg_t adc_chan_config = {
    .atten = ADC_ATTEN_DB_6,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
  };

  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle,ADC_CHANNEL_0, &adc_chan_config));
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle,ADC_CHANNEL_1, &adc_chan_config));

  //setup CAN
  CAN can = CAN(CANRX,CANTX);

  can.begin();

  double voltage = 0.1;
  while(1){
    vTaskDelay(pdMS_TO_TICKS(50));

  }

}