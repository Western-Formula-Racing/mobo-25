// Component - BMS
// Function: Open or close AMS relay depending on BMS state

#ifndef BMS_H
#define BMS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/twai.h"

struct Module{
  double voltage[20];
  double temp[18];
};

void clearFlags();
void errorCheckTask(void *pvParameters);
void updateCanTimeout(int time);
void setModuleVoltage(int module, int cell, double newVoltage);
void setModuleTemp(int module, int thermistor, double newTemp);
void updateCanTimeout(int time);
void setCurrent(double current);
void printModules();

#endif