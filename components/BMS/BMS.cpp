#include "BMS.h"

const char* TAG = "BMS";

void errorCheck(void *pvParameters){
  while(1){
    //Check every cell for open, under, or overvoltage and temps
    for(int i = 0;i<5;i++){
      for(int j = 0 ;j<20;j++){
        if(moboState.modules[i].voltage[j] == 0){
          moboState.flags.openCell = true;
          //ESP_LOGE(TAG,"Open Cell: Module %d Cell %d",i,j);
        }
        if(moboState.modules[i].voltage[j] > THRESHOLD_OVERVOLTAGE){
          moboState.flags.overVoltage = true;
          //ESP_LOGE(TAG,"Overvoltage: Module %d Cell %d",i,j);
        }
        if(moboState.modules[i].voltage[j] > THRESHOLD_UNDERVOLTAGE){
          moboState.flags.overVoltage = true;
          //ESP_LOGE(TAG,"Undervoltage: Module %d Cell %d",i,j);
        }
      }
      for(int k = 0 ;k<20;k++){
        if(moboState.modules[i].temp[k] == 0){
          moboState.flags.openThermistor = true;
          //ESP_LOGE(TAG,"Open Thermistor: Module %d Thermistor %d",i,k);
        }
        if(moboState.modules[i].temp[k] > THRESHOLD_OVERTEMP){
          moboState.flags.overVoltage = true;
          //ESP_LOGE(TAG,"Overtemp: Module %d Thermistor %d",i,k);
        }
      }
    }
    //check CAN timeout
    if(BMS_CAN_TIMEOUT_ENABLED && moboState.canTime > TEMP_VOLTAGE_TIMEOUT_MS ){
      moboState.flags.CANTimeout = true;
      ESP_LOGE(TAG,"CAN Timeout: %d ms", moboState.canTime);
    }
  
    if(moboState.getCurrent()>THRESHOLD_OVERCURRENT || moboState.getCurrent()<THRESHOLD_CHARGECURRENT){
      moboState.flags.overCurrent = true;
      ESP_LOGE(TAG,"Overcurrent detected: %.3fA",moboState.getCurrent());
    }

    //check flags for error, if yes then open AMS relay. If flags are clear AMS relay closes.
    if(moboState.flags.CANTimeout == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else if(moboState.flags.openCell == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else if(moboState.flags.openThermistor == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else if(moboState.flags.overCurrent == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else if(moboState.flags.overVoltage == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else if(moboState.flags.underVoltage == 1){
      gpio_set_level(GPIO_NUM_9,0);
    } else{
      gpio_set_level(GPIO_NUM_9,1);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }

};