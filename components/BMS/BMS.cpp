#include "BMS.h"

const char* TAG = "BMS";

Module modules[5];

void setModuleVoltage(int module, int cell, double newVoltage){
  if(module < 6 && cell <21){
    modules[module].voltage[cell] = newVoltage;
  } 
  else{ESP_LOGE(TAG, "Voltage out of range!");}
}

void setModuleTemp(int module, int thermistor, double newTemp){
  if(module < 6 && thermistor <19){
    modules[module].temp[thermistor] = newTemp;
  } 
  else{ESP_LOGE(TAG, "Temp out of range!");}
}

double getPackVoltage(){
  double total = 0;
  for(int i=0;i<5;i++){
    for(int j=0;j<20;j++){
      total += modules[i].voltage[j];
    }
  }
  return total;
}

double getMinTemp(){
  double min = 1000;
  for(int i=0;i<5;i++){
    for(int j=0;j<18;j++){
      if(min > modules[i].temp[j]){
        min = modules[i].temp[j];
      }
    }
  }
  return min;
}

double getMaxTemp(){
  double max = 0;
  for(int i=0;i<5;i++){
    for(int j=0;j<18;j++){
      if(max<modules[i].temp[j]){
        max = modules[i].temp[j];
      }
    }
  }
  return max;
}

double getMaxTemp(int& index){
  double max = 0;
  for(int i=0;i<5;i++){
    for(int j=0;j<18;j++){
      if(max<modules[i].temp[j]){
        max = modules[i].temp[j];
        index = i*18+j;
      }
    }
  }
  return max;
}

double getMaxVoltage(){
  double max = 0;
  for(int i=0;i<5;i++){
    for(int j=0;j<20;j++){
      if(max<modules[i].voltage[j]){ 
        max = modules[i].voltage[j];
      }
    }
  }
  return max;
}

double getMaxVoltage(int& index){
  double max = 0;
  for(int i=0;i<5;i++){
    for(int j=0;j<20;j++){
      if(max<modules[i].voltage[j]){ 
        max = modules[i].voltage[j];
        index = i*20+j;
      }
    }
  }
  return max;
}

double getMinVoltage(){
  double min = 100;
  for(int i=0;i<5;i++){
    for(int j=0;j<20;j++){
      if(min>modules[i].voltage[j]) min = modules[i].voltage[j];
    }
  }
  return min;
}

double getMinVoltage(int& index){
  double min = 100;
  for(int i=0;i<5;i++){
    for(int j=0;j<20;j++){
      if(min>modules[i].voltage[j]) {
        min = modules[i].voltage[j];
        index = i*20+j;
      }
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