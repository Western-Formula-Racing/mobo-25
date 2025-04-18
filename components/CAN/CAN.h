#ifndef CAN_H
#define CAN_H

#include <stdio.h>
#include <driver/twai.h>
#include "moboTasks.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "BMS.h"

#define CAN_TIMEOUT 10000 // milliseconds

class CAN{
  public:
    CAN(gpio_num_t rxpin, gpio_num_t txpin);
    void begin();
  private:
    int txCallbackCounter = 0;
    void txCallback();
    static void txCallbackWrapper(TimerHandle_t xTimer);
    void rxTask();
    static void rxTaskWrapper(void *arg);
    static void timeoutHandler(void *arg);

};

void canAlertTask(void *pvParameters);

#endif
