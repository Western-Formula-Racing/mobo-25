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
  uint8_t status;
  double moduleTemp;
  double lowestVoltage;
  uint8_t lowestIndex;
  double highestVoltage;
  uint8_t highestIndex;
  double voltage[20];
  double temp[18];
};
/* Error Codes:
  69 - Thermistor >60C
  70 - cell voltage <2.5 V
  71 - cell voltage >4.2 V
  72 - cell imbalance >0.2 V
  73 - open cell circuit
  74 - open thermistor circuit
  75 - LTC6813 DIAGN fail
  76 - LTC6813 AXST fail
  77 - LTC6813 CVST fail
  78 - LTC6813 STATST fail
  79 - LTC6813 ADOW fail
  80 - LTC6813 AXOW fail
  81 - LTC6813 ADOL fail
  82 - LTC6813 repeating CRC fail
  83 - Overcurrent fail
  84 - Can Timeout fail
  85 - CAN errors > 96
*/

struct errorFlags{
  bool errored = false;       //has an error been raised 
  //////////////////////////  //error-specific values:
  uint8_t moduleNumber = 0;       //module that raised the error 
  uint8_t errorCode = 0;          //error number
  double cellVoltage = 0;         //cell voltage value
  uint8_t cellIndex = 0;          //cell number
  double thermistorTemp = 0;      //thermistor temperature value
  uint8_t thermistorIndex = 0;    //thermistor number
  int timeoutTime = 0;        //CAN timeout period
  double timeoutCurrent = 0;  //amperage at fault time

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

//serial debugging
void printModules();

#endif