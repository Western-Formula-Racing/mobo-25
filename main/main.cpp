#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "config.h"
#include "esp_adc/adc_oneshot.h"


extern "C" void app_main(void)
{
  //setup

  //init inputs
  gpio_set_direction(BSPD_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(IMD_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(LATCH_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(HV_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(AIRN_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(CHARGE_PIN,GPIO_MODE_INPUT);

  //init outputs
  gpio_set_direction(AMS_LATCH,GPIO_MODE_INPUT_OUTPUT);
  gpio_set_level(AMS_LATCH,1);
  gpio_set_direction(PRECH_OK,GPIO_MODE_OUTPUT);
  gpio_set_level(PRECH_OK,0);

  //setup ADC
  adc_oneshot_unit_handle_t adc1_handle;

  adc_oneshot_unit_init_cfg_t adcconfig = {
    .unit_id = ADC_UNIT_1,
    .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
  };

  ESP_ERROR_CHECK(adc_oneshot_new_unit(&adcconfig, &adc1_handle));
  
  adc_oneshot_chan_cfg_t adc_chan_config = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
  };
  
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle,PRECHSENSE_ADC, &adc_chan_config));
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle,CURSENSE_ADC, &adc_chan_config));  

  while(1){
    printf("I AM ALIVE\n");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}


