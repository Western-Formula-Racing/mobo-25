#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "config.h"
#include "BMS.h"
#include "utils.h"
#include "CAN.h"

extern bool onChargeCart;

enum errorCode{
  NONE = 0,
  OVERTEMP = 69,  //Thermistor > 60C
  UNDERVOLTAGE,   //Cell voltage < 3V
  OVERVOLTAGE,    //cell voltage > 4.2 V
  IMBALANCE,      //cell imbalance > 0.2 V
  OPENCELL,       //open cell circuit
  OPENTHERMISTOR, //open thermistor circuit
  DIAGN,          //LTC6813 DIAGN fail
  AXST,           //LTC6813 AXST fail
  CVST,           //LTC6813 CVST fail
  STATST,         //LTC6813 STATST fail
  ADOW,           //LTC6813 ADOW fail
  AXOW,           //LTC6813 AXOW fail
  ADOL,           //LTC6813 ADOL fail
  CRCFAIL,        //LTC6813 repeating CRC fail
  OVERCURRENT,    //Overcurrent fail
  CANTIMEOUT,     //Can Timeout fail
  CANERROR,       //CAN errors > 96
  PRECHARGEFAIL   //Precharge took longer than 8 seconds
};

struct errorFlags{
  errorCode error = NONE;
  //////////////////////////  //error-specific values:
  uint8_t moduleNumber = -1;       //module that raised the error 
  double cellVoltage = -1;         //cell voltage value
  uint8_t cellIndex = -1;          //cell number
  double thermistorTemp = -1;      //thermistor temperature value
  uint8_t thermistorIndex = -1;    //thermistor number
  int32_t timeoutTime = -1;            //CAN timeout period
  int timeoutModule = -1;
  double timeoutCurrent = -1;      //current at fault time
};

enum state{
  IDLE,             //LV on
  PRECHARGE_START,  //Precharge start transition
  PRECHARGE_CHECK,  //Precharge check
  ACTIVE,           //HV Active
  CHARGING,  
  CHARGE_COMPLETE,
  FAULT
};

struct stateVars{
  state currentState = IDLE;
  state previousState = IDLE;
  uint32_t prechargeStartTime = 0;
  uint32_t prechargeEndTime = 0;
  bool airN = 0;
  bool chargePin = 1;
  errorFlags errors;
};

void stateLoop();
void checkFaults();
state getStatus();
stateVars getStateVariables();
errorCode getErrorCode();
void printFault(errorFlags errorflags);
void raiseError(errorFlags error);

#endif