#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "CAN.h"
#include "moboTasks.h"
#include "BMS.h"
#include "config.h"
#include "driver/ledc.h"


extern "C" void app_main(void)
{

  //tatic const char* TAG = "main";

  vTaskDelay(pdMS_TO_TICKS(1000));
  printf("Starting...");

  esp_log_level_set("*",ESP_LOG_INFO);

  //setup CAN
  CAN can = CAN(CANRX,CANTX);
  can.begin();

  moboSetup(); //setup i/o
  //xTaskCreatePinnedToCore(errorCheckTask,"Error Check",4096,NULL,configMAX_PRIORITIES-5,NULL,1); //start BMS task
  xTaskCreatePinnedToCore(inputTask, "inputTask", 4096, NULL,configMAX_PRIORITIES-6, NULL, 1); //start input task
  xTaskCreatePinnedToCore(prechargeTask,"prechargeTask",2048,NULL,configMAX_PRIORITIES-6,NULL,1);
  xTaskCreatePinnedToCore(coolingTask,"coolingTask",2048,NULL,configMAX_PRIORITIES-7,NULL,1);

  while(1){
    //printModules();

    vTaskDelay(pdMS_TO_TICKS(1000));
    ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0,100);
    ledc_update_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0,25);
    ledc_update_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    printf("heartbeat.\n");
  }
}


