#include "CAN.h"
static const char *TAG = "CAN";

uint32_t alerts_to_enable = TWAI_ALERT_ABOVE_ERR_WARN|TWAI_ALERT_ERR_ACTIVE|TWAI_ALERT_RX_QUEUE_FULL;
int txCounter = 0;
twai_status_info_t twai_status;

int32_t moduleTime[5] = {0,0,0,0,0};

twai_message_t txMessage = {

  // Message type and format settings
    .extd = 0,              // Standard vs extended format
    .rtr = 0,               // Data vs RTR frame
    .ss = 0,                // Whether the message is single shot (i.e., does not repeat on error)
    .self = 0,              // Whether the message is a self reception request (loopback)
    .dlc_non_comp = 0,      // DLC is less than 8
  // Message ID and payload
  .identifier = 1056,
  .data_length_code = 8,
  .data = {0,0,0,0,0,0,0,0},
};

// can instatiation

void CANbegin(gpio_num_t rxpin, gpio_num_t txpin){ 

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(txpin,rxpin,TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  
  ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
  ESP_LOGI(TAG, "CAN Driver Installed\n");
  ESP_ERROR_CHECK(twai_reconfigure_alerts(alerts_to_enable,NULL));
  // Start TWAI driver
  ESP_ERROR_CHECK(twai_start());
  //Start TX task
  TimerHandle_t tx_timer = xTimerCreate("CANtx", pdMS_TO_TICKS(10), pdTRUE, NULL, txTask);
  xTimerStart(tx_timer,0);
  //Start RX task
  xTaskCreatePinnedToCore(rxTask, "CAN_rx", 4096, NULL, 5, nullptr, 1);
  //xTaskCreatePinnedToCore(timeoutHandler, "timeoutHandler", 4096, NULL, 5, nullptr, 1);
}

void txTask(TimerHandle_t xTimer){
  //this function gets called every 10 milliseconds

  //10ms messages:

  if(txCounter%10 == 0){
    //100ms messages:
    txMessage.identifier = 1056;
    int16_t current = (int16_t) (getPackCurrent()*10);
    //printf("Scaled pack current:%d",current);
    current += 32760;
    txMessage.data[0] = (current & 0xFF);
    txMessage.data[1] = (current & 0xFF00)>>8;
    uint8_t imd = gpio_get_level(IMD_GPIO);
    uint8_t ams = gpio_get_level(AMS_LATCH);
    uint8_t bspd = gpio_get_level(BSPD_GPIO);
    uint8_t latch = gpio_get_level(LATCH_GPIO);
    uint8_t prech_en = gpio_get_level(AIRN_GPIO);
    uint8_t HVAct = gpio_get_level(HV_GPIO);
    txMessage.data[2] = (imd & (ams<<1) & (bspd<<2) & (latch<<3) & (prech_en<<4) & (HVAct<<5));
    uint16_t SOC = (uint16_t)(getSOC() * 100);
    txMessage.data[3] = (uint8_t)SOC & 0xFF;
    txMessage.data[4] = (uint8_t)(SOC & 0xFF00)>>8;
    txMessage.data[5] = getStatus();
    txMessage.data[6] = getErrorCode();
    txMessage.data[7] = 0;

    twai_transmit(&txMessage,portMAX_DELAY);
  }

  if(txCounter>=100){
    //1s messages
    if(getStatus() != CHARGING){
      elconControl(0,0,0);
    }
    else if(getStatus() == CHARGING){
      elconControl(MAX_CHARGE*100,CHARGE_CURRENT,1);
    }
    else if(getStatus() == CHARGE_COMPLETE){
      
      txMessage.data[0] = (int16_t)(getMinVoltage()*10000) & 0xFF;
      txMessage.data[1] = ((int16_t)(getMinVoltage()*10000) & 0x00FF)>>8;
      txMessage.data[2] = 0;
      txMessage.data[3] = 0;
      txMessage.data[4] = 0;
      txMessage.data[5] = 0;
      txMessage.data[6] = 0;
      txMessage.data[7] = 0;
    }
    txCounter = 0;
  }
  txCounter++;
}

void rxTask(void *arg){
  // create generic message
  twai_message_t rx_msg;
  ESP_LOGI(TAG,"Rx Task Started");
  while(1){
    // wait until a message is recieved
    if(twai_receive(&rx_msg,portMAX_DELAY) == ESP_OK){

      //ESP_LOGI(TAG,"Message recieved with ID: %ld", rx_msg.identifier);
      // Module voltages
      if(rx_msg.identifier >= 1006 && rx_msg.identifier <= 1030){
        double s1 = (rx_msg.data[0] | (int)rx_msg.data[1] << 8)*0.0001;
        double s2 = (rx_msg.data[2] | (int)rx_msg.data[3] << 8)*0.0001;
        double s3 = (rx_msg.data[4] | (int)rx_msg.data[5] << 8)*0.0001;
        double s4 = (rx_msg.data[6] | (int)rx_msg.data[7] << 8)*0.0001;
        int id = rx_msg.identifier-1006;
        int module = id/5;
        int cell = id*4 - module*20;
        //ESP_LOGI(TAG,"writing to module %d, id %d, starting cell %d", module, id, cell);
        setModuleVoltage(module,cell++,s1);
        setModuleVoltage(module,cell++,s2);
        setModuleVoltage(module,cell++,s3);
        setModuleVoltage(module,cell++,s4);
        moduleTime[module] = pdTICKS_TO_MS(xTaskGetTickCount());
      }
      //Module Temperatures
      else if(rx_msg.identifier >= 1031 && rx_msg.identifier <= 1055){
        
        double s1 = (rx_msg.data[0] | (int)rx_msg.data[1] << 8)*0.001;
        double s2 = (rx_msg.data[2] | (int)rx_msg.data[3] << 8)*0.001;
        double s3 = (rx_msg.data[4] | (int)rx_msg.data[5] << 8)*0.001;
        double s4 = (rx_msg.data[6] | (int)rx_msg.data[7] << 8)*0.001;
        int id = rx_msg.identifier-1031;
        int module = id/5;
        int cell = id*4 - module*20;

        setModuleTemp(module,cell++,s1);
        setModuleTemp(module,cell++,s2);
        if((id+1)%5!= 0){
          setModuleTemp(module,cell++,s3);
          setModuleTemp(module,cell++,s4);
        }
        //update last received for timeout
      }
      
      else if(rx_msg.identifier == 1000){
        //Error message!
        uint8_t module_id = rx_msg.data[0];
        errorCode errorcode = (errorCode)rx_msg.data[1];
        double voltage = (rx_msg.data[2] | (int)rx_msg.data[3] << 8)*0.001;
        uint8_t voltageIndex = rx_msg.data[4];
        double temperature = (rx_msg.data[2] | (int)rx_msg.data[3] << 8)*0.01;
        uint8_t tempIndex = rx_msg.data[7];

        errorFlags newError = {
          .error = errorcode,
          .moduleNumber = module_id,
          .cellVoltage = voltage,
          .cellIndex = voltageIndex,
          .thermistorTemp = temperature,
          .thermistorIndex = tempIndex,
        };
        raiseError(newError);
      }
    }
  }
}

void elconControl(double maxVoltage, double maxCurrent, bool enable){
  txMessage.extd = 1;
  txMessage.identifier = 403105268;
  uint16_t voltage = (maxVoltage*10);
  uint16_t current = (maxCurrent*10);

  txMessage.data[0] = ((voltage & 0xFF00)>>8);
  txMessage.data[1] = (voltage & 0xFF);
  txMessage.data[2] = ((current & 0xFF00)>>8);
  txMessage.data[3] = (current & 0xFF);
  txMessage.data[4] = (!enable);
  txMessage.data[5] = 0;
  txMessage.data[6] = 0;
  txMessage.data[7] = 0;
  
  twai_transmit(&txMessage,portMAX_DELAY);
  txMessage.extd = 0;
}

void balanceMessage(){
  uint16_t minVoltage = getMinVoltage()*10000;
  txMessage.identifier = 999;
  txMessage.data[0] = (uint8_t)(minVoltage & 0x0F);
  txMessage.data[1] = (uint8_t)(minVoltage & 0xF0)>>8;
  twai_transmit(&txMessage,portMAX_DELAY);
}

int32_t getMaxCanTimeout(){
  int32_t timeout = 0;
  for(int i =0;i<5;i++){
    if(pdTICKS_TO_MS(xTaskGetTickCount())-moduleTime[i]>timeout){
      //printf("Module %d time: %ld",i,moduleTime[i]);
      timeout = pdTICKS_TO_MS(xTaskGetTickCount())-moduleTime[i];
    }
  }
  //printf("Current task time: %ld\n",pdTICKS_TO_MS(xTaskGetTickCount()));
  return timeout;
};

uint32_t getCANErrorCount(){
  twai_get_status_info(&twai_status);
  return twai_status.bus_error_count;
}

int32_t getModuleTime(int module){
  return moduleTime[module];
}