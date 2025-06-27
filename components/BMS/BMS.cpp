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

//SoC Code 

double setHallCalibrationInverter(float hallZero){

//if inverter DC current in = 0 & inverter status message is active; then hallZero is equal to whatever CURSENSE_ADC is; if inverter is on just give the last value 
  
}

double setHallCalibrationElCon(float hallZero){

//When ElCon current ouput = 0 & ElCon status message is active; then hallZero is equal to whatever CURSENSE_ADC is; if ElCon is on just give the last value 
  
}

double getMaxCharge(double maxCharge, double currentCharge){
//if getPackVoltage() is 300-305 V and ElCon output current is greater then zero then enter a loop that adds to a variable, when the ElCon output goes to zero exit the loop and decide wether to 
//add the total charge to just currentCharge or both currentCharge and MaxCharge based on if we charged the back up to 405-410V (set them equal and return one of them) 

}


double getCurrentFlow(double currentFlow){

//call on your hallZero functions based on which status message you are recieving (ElCon or Cascadia) then based on the calibration convert the CURSENSE input into a current and return currentFLow

}

double getCurrentCharge(double currentCharge){
  //loop through calling your current flow function every 100ms or so and subtract it from whatever your current charge; you must find a way to return the currentCharge inn the getMaxCharge was initially
}
double getSoC(double SoC, double currentCharge, double maxCharge){
  //use your currentCharge and maxCharge to see what SoC you are at
  
}
