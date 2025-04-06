#ifndef TASKS_H
#define TASKS_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "stdio.h"
#include "config.h"
#include "driver/gpio.h"
#include "BMS.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/ledc.h"

enum state{
  IDLE = 0,
  PRECHARGE,
  ACTIVE,
  CHARGING,
  FAULT
};

void moboSetup();
state getStatus();
void prechargeTask(void *pvParameters);
void inputTask(void *pvParameters);
void coolingTask(void *pvParameters);
void telemetryTask(void *pvParameters);

#endif