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

  //tatic const char* TAG = "main";

  vTaskDelay(pdMS_TO_TICKS(1000));
  printf("Starting...");

  esp_log_level_set("*",ESP_LOG_INFO);

  //setup CAN
  CAN can = CAN(CANRX,CANTX);

  can.begin();
  moboSetup();
  // Start error checking task
  //xTaskCreatePinnedToCore(errorCheck, "errorCheck", 4096, nullptr, 1, nullptr, 1);
  while(1){
    vTaskDelay(pdMS_TO_TICKS(1000));
    //moboState.resetFlags();
    //moboState.printModules();
    //printf("BMS Relay State: %d\n\n",gpio_get_level(AMS_LATCH));
    //printf("%d %d %d %d",moboState.flags.underVoltage,moboState.flags.CANTimeout,moboState.flags.overCurrent,moboState.flags.openCell);
  }
}


