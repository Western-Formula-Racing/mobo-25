// Component - BMS
// Function: Track BMS State
#ifndef BMS_H
#define BMS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/twai.h"

//module values
struct Module{
  double voltage[20];
  double temp[18];
};

// functions used for CAN input (setters):
void setModuleVoltage(int module, int cell, double newVoltage);
void setModuleTemp(int module, int thermistor, double newTemp);

// info for other functions
double getPackVoltage();  // sum of all cell voltages
double getMaxTemp();      // maximum thermistor temperature
double getMinTemp();
double getMaxTemp(int& index);
double getMaxVoltage();   // maximum cell voltage
double getMaxVoltage(int& index);
double getMinVoltage();   // minimum cell voltage
double getMinVoltage(int& index);

//serial debugging
void printModules();

#endif