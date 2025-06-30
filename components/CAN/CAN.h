#ifndef CAN_H
#define CAN_H

#include <stdio.h>
#include <driver/twai.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "BMS.h"
#include "statemachine.h"
#include "utils.h"



void CANbegin(gpio_num_t rxpin, gpio_num_t txpin);

void txTask(TimerHandle_t xTimer);
void rxTask(void *arg);
//static void timeoutHandler(void *arg);
extern int txCounter;


//void canAlertTask(void *pvParameters);
void elconControl(double maxVoltage, double maxCurrent, bool enable);
void balanceMessage();
int32_t getMaxCanTimeout();
uint32_t getCANErrorCount();
int32_t getModuleTime(int module);

#endif
