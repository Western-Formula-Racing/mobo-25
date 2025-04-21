#include "BMS.h"

const char* TAG = "BMS";

errorFlags errors;            //error state tracking
double packCurrent = 0.0;     //amps from current sensor
uint32_t canTime[5] = {0,0,0,0,0};         //time since last BMS data message, in ms
Module modules[5] = {};       //module voltages and temps

// functions used for CAN input (setters):

void setModuleVoltage(int module, int cell, double newVoltage){
  modules[module].voltage[cell] = newVoltage;
};

void setModuleTemp(int module, int thermistor, double newTemp){
  modules[module].temp[thermistor] = newTemp;
};

void updateCanTimeout(int index, uint32_t time){
  canTime[index] = time;
}

// functions used for other input:

void setCurrent(double current){
  packCurrent = current;
}

// Error tracking and raising:

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
    case 84:
      ESP_LOGE(TAG,"Error: Overcurrent: %.2f A",errors.timeoutCurrent);
      break;
    case 83:
      ESP_LOGE(TAG,"Error: Can Timeout from module: %d",errors.moduleNumber);
      break;
    case 85:
      ESP_LOGE(TAG, "Error: Canbus Disconnected");
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

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
    //Check error struct for errors from elsewhere (BMS CAN message, etc):
    if(errors.errored == true){
      raiseError();
    }
  
    //check overcurrent
    if(packCurrent>THRESHOLD_OVERCURRENT || packCurrent<THRESHOLD_CHARGECURRENT){
      errors.errored = true;
      errors.errorCode = 84;
      errors.timeoutCurrent = packCurrent;
      raiseError();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
};

// info for other functions

double getPackCurrent(){
  return packCurrent;
}

double getSOC(){
  return getPackVoltage();
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

double getPackVoltage(){
  double voltage = 0;
  for(int i =0;i<5;i++){
    for(int j = 0;j<20;j++){
      voltage += modules[i].voltage[j];
    }
  }
  return voltage;
}

double getMaxVoltage(){
  double max = 0;
  for(int i =0;i<5;i++){
    for(int j = 0;j<20;j++){
      if(modules[i].voltage[j]>max) max = modules[i].voltage[j];
    }
  }
  return max;
}

double getMinVoltage(){
  double min = 100;
  for(int i =0;i<5;i++){
    for(int j = 0;j<20;j++){
      if(modules[i].voltage[j]<min) min = modules[i].voltage[j];
    }
  }
  return min;
}
//serial debugging

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
    printf("\n");
  }
}