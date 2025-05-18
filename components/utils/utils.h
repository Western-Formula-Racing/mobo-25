#ifndef UTILS_H
#define UTILS_H

#include "esp_adc/adc_oneshot.h"
#include "statemachine.h"
#include "CAN.h"

void setupADC();
double getPackCurrent();
double getPrechargeVoltage();
double getSOC();
void printInfo();

#endif