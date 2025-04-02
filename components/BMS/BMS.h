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
*/

struct errorFlags{
  bool errored = 0;
  uint8_t moduleNumber;
  uint8_t errorCode;
  double cellVoltage;
  uint8_t cellIndex;
  double thermistorTemp;
  uint8_t thermistorIndex;
  int timeoutTime = 0;
  double timeoutCurrent = 0;

};

void setErrorFlags(errorFlags newError);
errorFlags getErrorFlags();
void errorCheckTask(void *pvParameters);
void updateCanTimeout(int time);
void setModuleVoltage(int module, int cell, double newVoltage);
void setModuleTemp(int module, int thermistor, double newTemp);
void updateCanTimeout(int time);
void setCurrent(double current);
void printModules();
void raiseError();
double getMaxTemp();

#endif