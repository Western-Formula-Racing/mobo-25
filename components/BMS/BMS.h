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

//module values
struct Module{
  double voltage[20];
  double temp[18];
};

// functions used for CAN input (setters):
void setModuleVoltage(int module, int cell, double newVoltage);
void setModuleTemp(int module, int thermistor, double newTemp);
void updateCanTimeout(int index, uint32_t time);

// functions used for other input:
void setCurrent(double current);

// Error tracking and raising:
void raiseError(); // Opens AMS relay, spits error on serial
void setErrorFlags(errorFlags newError); //setter
errorFlags getErrorFlags(); //getter
void errorCheckTask(void *pvParameters); //background error detection task

// info for other functions
double getPackCurrent();
double getMaxTemp();
double getSOC();
double getPackVoltage();
double getMaxVoltage();
double getMinVoltage();

//serial debugging
void printModules();

#endif