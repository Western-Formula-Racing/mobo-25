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

void moboSetup();
void prechargeTask();
void inputTask(void *pvParameters);

#endif