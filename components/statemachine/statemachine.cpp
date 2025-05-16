#include "statemachine.h"

static const char *TAG = "State Machine";

stateVars stateLoop(stateVars previous){

  stateVars next = previous;

  switch(previous.currentState){ 
    case IDLE:
    //check if precharge has started
    if(gpio_get_level(AIRN_GPIO)==1){
      next.previousState = IDLE;
      next.currentState = PRECHARGE_START;
    }
    checkFaults(&next);
    break;
    
    case PRECHARGE_START:
    //set start time
    next.prechargeStartTime =  pdTICKS_TO_MS(xTaskGetTickCount());
    next.previousState = PRECHARGE_START;
    next.currentState = PRECHARGE_CHECK;
    
    checkFaults(&next);
    break;
    
    case PRECHARGE_CHECK:
    //check if precharge voltage is fine, and if at least 2 seconds have passed. (calculated precharge time on ESF is 0.6s)
    if(getPrechargeVoltage()>(getPackVoltage()*0.9) && pdTICKS_TO_MS(xTaskGetTickCount()) - next.prechargeStartTime > 2000){
      next.previousState = PRECHARGE_CHECK;
      next.currentState = ACTIVE;
      gpio_set_level(PRECH_OK,1);
    }
    
    //check if time elapsed >8s
    if((pdTICKS_TO_MS(xTaskGetTickCount())-next.prechargeStartTime)>8000){
      next.previousState = PRECHARGE_CHECK;
      next.currentState = FAULT;
      next.errors.error = PRECHARGEFAIL;
    }

    checkFaults(&next);
    break;

    case ACTIVE:
    if(gpio_get_level(CHARGE_PIN) == 0){
      next.previousState = ACTIVE;
      next.currentState = CHARGING;
    }

    checkFaults(&next);
    break;
    
    case CHARGING:
    if(gpio_get_level(CHARGE_PIN) == 1){
      next.previousState = CHARGING;
      next.currentState = ACTIVE;
    }

    checkFaults(&next);
    break;
    
    case FAULT:
    
    switch (next.errors.error){
      case OVERTEMP:
      ESP_LOGE(TAG,"ERROR - Thermistor overtemp: %.2f");
    }

    break;
  }
  return next;
};

void checkFaults(stateVars* currentState){
}
