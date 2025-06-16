#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "config.h"
#include "esp_adc/adc_oneshot.h"
#include "statemachine.h"

extern "C" void app_main(void)
{
  static const char* TAG = "Main"; 
  bool blink = 1;
  uint32_t loopCounter=0;
  //setup

  //init inputs
  gpio_set_direction(BSPD_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(IMD_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(LATCH_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(HV_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(AIRN_GPIO,GPIO_MODE_INPUT);
  gpio_set_direction(CHARGE_PIN,GPIO_MODE_INPUT);
  gpio_set_pull_mode(CHARGE_PIN,GPIO_PULLUP_ONLY);
  gpio_set_direction(GPIO_NUM_0,GPIO_MODE_INPUT);
  
  //init outputs
  gpio_set_direction(STATUS_LED,GPIO_MODE_OUTPUT);
  gpio_set_direction(FAULT_LED,GPIO_MODE_OUTPUT);
  gpio_set_direction(AMS_LATCH,GPIO_MODE_INPUT_OUTPUT);
  gpio_set_direction(PRECH_OK,GPIO_MODE_INPUT_OUTPUT);
  gpio_set_direction(TSSI_ENABLE,GPIO_MODE_OUTPUT);
  gpio_set_level(PRECH_OK,0);
  gpio_set_level(TSSI_ENABLE,1);
  
  setupADC();
  CANbegin(CANRX,CANTX);
  vTaskDelay(pdMS_TO_TICKS(3000));
  gpio_set_level(AMS_LATCH,1);
  gpio_set_level(TSSI_ENABLE,0);
  
  while(1){
    if(loopCounter % 1 ==0){
      ESP_LOGI(TAG,"I AM ALIVE\n");
      printInfo();
    }
    stateLoop();
    getPrechargeVoltage();
    gpio_set_level(STATUS_LED,blink);
    blink = !blink;
    loopCounter++;
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}


