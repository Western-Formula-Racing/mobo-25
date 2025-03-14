#include "BMS.h"

const char* TAG = "BMS";

bool overVoltage = 0;
bool underVoltage = 0;
bool overTemp = 0;
bool CANTimeout = 0;
bool overCurrent = 0;
bool openCell = 0;
bool openThermistor = 0;

double packCurrent = 0.0;

uint32_t canTime = 0;

Module modules[5] = {};

//BMS consists of 2 tasks: recieve CAN signals and update variables, and check for errors, opening the relay if an error is detected.

void clearFlags(){
overVoltage = 0;
underVoltage = 0;
overTemp = 0;
CANTimeout = 0;
overCurrent = 0;
openCell = 0;
openThermistor = 0;
}

void startBMS(){
  xTaskCreatePinnedToCore(errorCheckTask,"Error Check",4096,NULL,configMAX_PRIORITIES-5,NULL,1);
}


void errorCheckTask(void *pvParameters){
  while(1){
    //Check every cell for open, under, or overvoltage and temps
    for(int i = 0;i<5;i++){
      for(int j = 0 ;j<20;j++){
        if(modules[i].voltage[j] == 0){
          openCell = true;
          ESP_LOGE(TAG,"Open Cell: Module %d Cell %d",i,j);
        }
        if(modules[i].voltage[j] > THRESHOLD_OVERVOLTAGE){
          overVoltage = true;
          ESP_LOGE(TAG,"Overvoltage: Module %d Cell %d",i,j);
        }
        if(modules[i].voltage[j] < THRESHOLD_UNDERVOLTAGE){
          underVoltage = true;
          ESP_LOGE(TAG,"Undervoltage: Module %d Cell %d",i,j);
        }
      }
      for(int k = 0 ;k<18;k++){
        if(modules[i].temp[k] == 0){
          openThermistor = true;
          ESP_LOGE(TAG,"Open Thermistor: Module %d Thermistor %d",i,k);
        }
        if(modules[i].temp[k] > THRESHOLD_OVERTEMP){
          overTemp = true;
          ESP_LOGE(TAG,"Overtemp: Module %d Thermistor %d",i,k);
        }
      }
    }
    //check CAN timeout
    if(BMS_CAN_TIMEOUT_ENABLED && canTime > TEMP_VOLTAGE_TIMEOUT_MS ){
      CANTimeout = true;
      ESP_LOGE(TAG,"CAN Timeout: %ld ms", canTime);
    }
    if(packCurrent>THRESHOLD_OVERCURRENT || packCurrent<THRESHOLD_CHARGECURRENT){
      overCurrent = true;
      ESP_LOGE(TAG,"Overcurrent detected: %.3fA",packCurrent);
    }
    //check for error, if yes then open AMS relay. If they are clear AMS relay closes.
    if(CANTimeout == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else if(openCell == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else if(openThermistor == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else if(overCurrent == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else if(overVoltage == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else if(underVoltage == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else{
      gpio_set_level(GPIO_NUM_9,1);
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