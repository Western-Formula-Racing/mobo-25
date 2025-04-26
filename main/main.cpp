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


extern "C" void app_main(void)
{

  static const char* TAG = "main";

  errorFlags emptyError;
  TaskHandle_t inputTaskHandle;
  TaskHandle_t prechargeTaskHandle;
  TaskHandle_t coolingTaskHandle;

  printf("Starting...");
  vTaskDelay(pdMS_TO_TICKS(3000)); // wait for serial to connect

  esp_log_level_set("*",ESP_LOG_INFO);
  
  //setup i/o
  moboSetup(); 

  //setup CAN
  CAN can = CAN(CANRX,CANTX);
  can.begin();

  xTaskCreatePinnedToCore(errorCheckTask,"Error Check",4096,NULL,configMAX_PRIORITIES-5,NULL,1); //start BMS task
  xTaskCreatePinnedToCore(canAlertTask,"CAN Alerts",2048,NULL,configMAX_PRIORITIES-5,NULL,1); //start CAN Alert watchdog
  xTaskCreatePinnedToCore(inputTask, "inputTask", 4096, NULL,configMAX_PRIORITIES-6, &inputTaskHandle, 1); //start input task
  xTaskCreatePinnedToCore(prechargeTask,"prechargeTask",2048,NULL,configMAX_PRIORITIES-6,&prechargeTaskHandle,1);
  xTaskCreatePinnedToCore(coolingTask,"coolingTask",2048,NULL,configMAX_PRIORITIES-7,&coolingTaskHandle,1);
  xTaskCreatePinnedToCore(telemetryTask,"telemetryTask",8192,NULL,configMAX_PRIORITIES-10,NULL,1);

  while(1){

    if(getErrorFlags().errored == 0){
        ESP_LOGI(TAG,"heartbeat.\n"); 
      }
    else{
      vTaskDelete(inputTaskHandle);
      vTaskDelete(prechargeTaskHandle);
      vTaskDelete(coolingTaskHandle);
      while(1){
        ESP_LOGI(TAG,"heart is no longer beating...\n"); 
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}


