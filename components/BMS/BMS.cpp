#include "BMS.h"

const char* TAG = "BMS";

errorFlags errors = {};            //error state tracking
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