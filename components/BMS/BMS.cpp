#include "BMS.h"

const char* TAG = "BMS";

errorFlags errors; 

double packCurrent = 0.0;

uint32_t canTime = 0;

Module modules[5] = {};

void setErrorFlags(errorFlags newError){
  errors = newError;
}

errorFlags getErrorFlags(){
  return errors;
}

void errorCheckTask(void *pvParameters){
  //3 second delay to make sure all CAN messages have been recieved, so errors don't trigger because values haven't been populated yet.
  vTaskDelay(pdMS_TO_TICKS(3000));
  while(1){
    //Check CAN error struct for errors:
    if(errors.errored == true){
      raiseError();
    }
    
    //check CAN timeout
    if(BMS_CAN_TIMEOUT_ENABLED && canTime > TEMP_VOLTAGE_TIMEOUT_MS ){
      errors.errored = true;
      errors.errorCode = 82;
      errors.timeoutTime = canTime;
      raiseError();
    }
    //check overcurrent
    if(packCurrent>THRESHOLD_OVERCURRENT || packCurrent<THRESHOLD_CHARGECURRENT){
      errors.errored = true;
      errors.errorCode = 83;
      errors.timeoutCurrent = packCurrent;
      raiseError();
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }

};

void setModuleVoltage(int module, int cell, double newVoltage){
  modules[module].voltage[cell] = newVoltage;
};

void setModuleTemp(int module, int thermistor, double newTemp){
  modules[module].temp[thermistor] = newTemp;
};

void updateCanTimeout(int time){
  canTime = time;
}

void setCurrent(double current){
  packCurrent = current;
}

void printModules(){
  printf("===Module Info===\n");
  printf("   Module 1 --- Cells        Module 2 --- Cells        Module 3 --- Cells        Module 4 --- Cells        Module 5 --- Cells     \n");
  for(int k=0;k<5;k++){
    for(int j=0; j<5;j++){
      for(int i =0;i<4;i++){
        printf("|%.3f",modules[j].voltage[i+k*4]);
      }
      printf("| ");
    }
    printf("\n");
  }
  printf("   Module 1 --- Temp        Module 2 --- Temp        Module 3 --- Temp        Module 4 --- Temp        Module 5 --- Temp     \n");
  for(int k=0;k<5;k++){
    for(int j=0; j<5;j++){
      for(int i =0;i<4;i++){
        if(i+k*4<18){
          printf("|%.3f",modules[j].temp[i+k*4]);
        } else{printf("|x.xxx");}
      }
      printf("| ");
    }
  }
};

void raiseError(){
  gpio_set_level(GPIO_NUM_9,0);
  while(1){
    switch (errors.errorCode)
    {
    case 69:
      ESP_LOGE(TAG,"Error: Thermistor %d overtemp: %.2f degC",errors.thermistorIndex,errors.thermistorTemp);
      break;
    case 70:
      ESP_LOGE(TAG,"Error: Cell %d undervoltage: %.2f V",errors.cellIndex,errors.cellVoltage);
      break;
    case 71:
      ESP_LOGE(TAG,"Error: Cell %d overvoltage: %.2f V",errors.cellIndex,errors.cellVoltage);
      break;
    case 72:
      ESP_LOGE(TAG,"Error: Cell Imbalance >0.2V");
      break;
    case 73:
      ESP_LOGE(TAG,"Error: Open Cell %d",errors.cellIndex);
      break;
    case 74:
      ESP_LOGE(TAG,"Error: Open Thermistor %d",errors.thermistorIndex);
      break;
    case 75:
      ESP_LOGE(TAG,"Error: LTC6813 DIAGN Fail");
      break;
    case 76:
      ESP_LOGE(TAG,"Error: LTC6813 AXST Fail");
      break;
    case 77:
      ESP_LOGE(TAG,"Error: LTC6813 CVST Fail");
      break;
    case 78:
      ESP_LOGE(TAG,"Error: LTC6813 STATST Fail");
      break;
    case 79:
      ESP_LOGE(TAG,"Error: LTC6813 ADOW Fail");
      break;
    case 80:
      ESP_LOGE(TAG,"Error: LTC6813 AXOW Fail");
      break;
    case 81:
      ESP_LOGE(TAG,"Error: LTC6813 ADOL Fail");
      break;
    case 82:
      ESP_LOGE(TAG,"Error: LTC6813 CRC Fail");
      break;
    case 83:
      ESP_LOGE(TAG,"Error: Overcurrent: %.2f A",errors.timeoutCurrent);
      break;
    case 84:
      ESP_LOGE(TAG,"Error: Can Timeout: %d ms",errors.timeoutTime);
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

double getMaxTemp(){
  double max = 0;
  for(int i =0;i<5;i++){
    for(int j = 0;j<18;j++){
      if(modules[i].temp[j]>max) max = modules[i].temp[j]; 

    }
  }
  return max;
}