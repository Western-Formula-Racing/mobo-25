#include "statemachine.h"

static const char *TAG = "State Machine";

stateVars stateVariables;
stateVars previousStateVariables;

int chargePinBuffer[5] = {1,1,1,1,1};
int chargePinBufferCount = 0;
int airNBuffer[5] = {0,0,0,0,0};
int airNBufferCount = 0;

void stateLoop(void){

  previousStateVariables = stateVariables;
  
  chargePinBuffer[chargePinBufferCount++] = gpio_get_level(CHARGE_PIN);
  if(chargePinBufferCount > 4) chargePinBufferCount = 0;
  if((chargePinBuffer[0] == 1) && (chargePinBuffer[1] == 1) && (chargePinBuffer[2] == 1) && (chargePinBuffer[3] == 1) && (chargePinBuffer[4] == 1)){
    stateVariables.chargePin = true;
  } else if((chargePinBuffer[0] == 0) && (chargePinBuffer[1] == 0) && (chargePinBuffer[2] == 0) && (chargePinBuffer[3] == 0) && (chargePinBuffer[4] == 0)){
    stateVariables.chargePin = false;
  }
  
  airNBuffer[airNBufferCount++] = gpio_get_level(AIRN_GPIO);
  if(airNBufferCount > 4) airNBufferCount = 0;
  if((airNBuffer[0] == 1) && (airNBuffer[1] == 1) && (airNBuffer[2] == 1) && (airNBuffer[3] == 1) && (airNBuffer[4] == 1)){
    stateVariables.airN = true;
  } else if ((airNBuffer[0] == 0) && (airNBuffer[1] == 0) && (airNBuffer[2] == 0) && (airNBuffer[3] == 0) && (airNBuffer[4] == 0)){
    stateVariables.airN = false;
  }
  
  
  switch(previousStateVariables.currentState){ 
    case IDLE:
    //check if precharge has started
    if(gpio_get_level(AIRN_GPIO)==1){
      stateVariables.previousState = IDLE;
      stateVariables.currentState = PRECHARGE_START;
    }
    checkFaults();
    break;
    
    case PRECHARGE_START:
    //set start time
    stateVariables.prechargeStartTime =  pdTICKS_TO_MS(xTaskGetTickCount());
    stateVariables.previousState = PRECHARGE_START;
    stateVariables.currentState = PRECHARGE_CHECK;
    
    checkFaults();
    break;
    
    case PRECHARGE_CHECK:
    //check if precharge voltage is fine, and if at least 2 seconds have passed. (calculated precharge time on ESF is 0.6s)
    if(getPrechargeVoltage()>(getPackVoltage()*0.9) && pdTICKS_TO_MS(xTaskGetTickCount()) - stateVariables.prechargeStartTime > 2000){
      stateVariables.previousState = PRECHARGE_CHECK;
      stateVariables.currentState = ACTIVE;
      gpio_set_level(PRECH_OK,1);
    }
    
    //check if time elapsed >8s
    if((pdTICKS_TO_MS(xTaskGetTickCount())-stateVariables.prechargeStartTime)>8000){
      stateVariables.previousState = PRECHARGE_CHECK;
      stateVariables.currentState = FAULT;
      stateVariables.errors.error = PRECHARGEFAIL;
    }

    checkFaults();
    break;

    case ACTIVE:

    
    if(gpio_get_level(CHARGE_PIN) == 0 && onChargeCart == true){
      stateVariables.previousState = ACTIVE;
      stateVariables.currentState = CHARGING;
    }
    
    
    if(stateVariables.airN == 0){
      vTaskDelay(pdMS_TO_TICKS(10));
      if(stateVariables.airN == 0){
      stateVariables.previousState = ACTIVE;
      stateVariables.currentState = IDLE;
      gpio_set_level(PRECH_OK,0);
      }
    }
    checkFaults();
    break;
    
    case CHARGING:
    if(stateVariables.chargePin == 1){
      stateVariables.previousState = CHARGING;
      stateVariables.currentState = ACTIVE;
    }
    if(getMaxVoltage() >= MAX_CHARGE){
      stateVariables.previousState = CHARGING;
      stateVariables.currentState = CHARGE_COMPLETE;
      ESP_LOGI(TAG,"Charging complete; Waiting 10 seconds...");
      for(int i = 0; i<10;i++){
        ESP_LOGI(TAG,"%d seconds...",i);
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
      ESP_LOGI(TAG,"Lowest Voltage: %.3f",getMinVoltage());
      
    }

    checkFaults();
    break;

    case CHARGE_COMPLETE:
    ESP_LOGE(TAG,"Charging Complete");
    checkFaults();
    break;

    case FAULT:
    gpio_set_level(FAULT_LED,1);
    gpio_set_level(AMS_LATCH,0);
    printFault(stateVariables.errors);

    break;
  }
};

state getStatus(){
  return stateVariables.currentState;
}
stateVars getStateVariables(){
  return stateVariables;
}
errorCode getErrorCode(){
  return stateVariables.errors.error;
}

void checkFaults(){
  int index;
  if(getMaxTemp()>THRESHOLD_OVERTEMP){
    stateVariables.errors.error = OVERTEMP;
    stateVariables.previousState = stateVariables.currentState;
    stateVariables.currentState = FAULT;
    stateVariables.errors.thermistorTemp = getMaxTemp(index);
    stateVariables.errors.thermistorIndex = index;
  }
  else if(getMinVoltage()<THRESHOLD_UNDERVOLTAGE && getMinVoltage() != 0){
    stateVariables.errors.error = UNDERVOLTAGE;
    stateVariables.previousState = stateVariables.currentState;
    stateVariables.currentState = FAULT;
    stateVariables.errors.cellVoltage = getMinVoltage(index);
    stateVariables.errors.cellIndex = index;
  }
  else if(getMaxVoltage()>THRESHOLD_OVERVOLTAGE){
    stateVariables.errors.error = OVERVOLTAGE;
    stateVariables.previousState = stateVariables.currentState;
    stateVariables.currentState = FAULT;
    stateVariables.errors.cellVoltage = getMaxVoltage(index);
    stateVariables.errors.cellIndex = index;
  }
  else if(getMaxVoltage()-getMinVoltage()>THRESHOLD_MAXDELTA && getMinVoltage() > 0 && stateVariables.currentState == IDLE){
    stateVariables.errors.error = IMBALANCE;
    stateVariables.previousState = stateVariables.currentState;
    stateVariables.currentState = FAULT;
    stateVariables.errors.cellVoltage = getMaxVoltage()-getMinVoltage();
  }
  //open cell
  //open thermistor
  //LTC errors
  else if(getPackCurrent()>THRESHOLD_OVERCURRENT){
    stateVariables.errors.error = OVERCURRENT;
    stateVariables.previousState = stateVariables.currentState;
    stateVariables.currentState = FAULT;
    stateVariables.errors.timeoutCurrent = getPackCurrent();
  }
  else if(getMaxCanTimeout()>THRESHOLD_CANTIMEOUT){
    stateVariables.errors.error = CANTIMEOUT;
    stateVariables.previousState = stateVariables.currentState;
    stateVariables.currentState = FAULT;
    stateVariables.errors.timeoutTime = getMaxCanTimeout();
  }
  else if(getCANErrorCount()>9000){
    stateVariables.errors.error = CANERROR;
    stateVariables.previousState = stateVariables.currentState;
    stateVariables.currentState = FAULT;
  }
}

void printFault(errorFlags errorflags){
  switch(errorflags.error){
    case NONE:
      ESP_LOGE(TAG,"No Fault");
    break;
    case OVERTEMP:
      ESP_LOGE(TAG,"Fault: Overtemp - %.2fC | Thermistor %d\n",errorflags.thermistorTemp,errorflags.thermistorIndex);
    break;
    case UNDERVOLTAGE:
      ESP_LOGE(TAG,"Fault: Undervoltage - %.2fV | Cell %d\n",errorflags.cellVoltage, errorflags.cellIndex);
    break;
    case OVERVOLTAGE:
      ESP_LOGE(TAG,"Fault: Overvoltage - %.2fV | Cell %d\n",errorflags.cellVoltage, errorflags.cellIndex);
    break;
    case IMBALANCE:
      ESP_LOGE(TAG,"Fault: Cells out of balance - %.2fV\n",errorflags.cellVoltage);
    break;
    case OPENCELL:
      ESP_LOGE(TAG,"Fault: Open Cell - %d\n",errorflags.cellIndex);
    break;
    case OPENTHERMISTOR:
      ESP_LOGE(TAG,"Fault: Open Thermistor - %d\n", errorflags.thermistorIndex);
    break;
    case DIAGN:
      ESP_LOGE(TAG,"LTC DIAGN Error\n");
    break;
    case AXST:
      ESP_LOGE(TAG,"LTC AXST Error\n");
    break;
    case CVST:
      ESP_LOGE(TAG,"LTC CVST Error\n");
    break;
    case STATST:
      ESP_LOGE(TAG,"LTC STATST Error\n");
    break;
    case ADOW:
      ESP_LOGE(TAG,"LTC ADOW Error\n");
    break;
    case AXOW:
      ESP_LOGE(TAG,"LTC AXOW Error\n");
    break;
    case ADOL:
      ESP_LOGE(TAG,"LTC ADOL Error\n");
    break;
    case CRCFAIL:
      ESP_LOGE(TAG,"LTC CRCFAIL Error\n");
    break;
    case OVERCURRENT:
      ESP_LOGE(TAG,"Fault: Overcurrent - %.2fA\n",errorflags.timeoutCurrent);
    break;
    case CANTIMEOUT:
      ESP_LOGE(TAG,"Fault: CAN Timeout - %ld ms - Module %d\n",errorflags.timeoutTime,errorflags.timeoutModule);
    break;
    case CANERROR:
      ESP_LOGE(TAG,"Fault: Too many CAN errors - %ld\n",getCANErrorCount());
    break;
    case PRECHARGEFAIL:
      ESP_LOGE(TAG,"Precharge Failed!");
    break;
}
}

void raiseError(errorFlags error){
  stateVariables.errors = error;
  stateVariables.previousState = stateVariables.currentState;
  stateVariables.currentState = FAULT;
}