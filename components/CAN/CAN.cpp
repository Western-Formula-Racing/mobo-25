#include "CAN.h"
#include "CANSignals.h"

static const char *TAG = "CAN";

twai_message_t txmessage = {

  // Message type and format settings
  .extd = 1,              // Standard vs extended format
  .rtr = 0,               // Data vs RTR frame
  .ss = 0,                // Whether the message is single shot (i.e., does not repeat on error)
  .self = 0,              // Whether the message is a self reception request (loopback)
  .dlc_non_comp = 0,      // DLC is less than 8
  
  // Message ID and payload
  .identifier = 0xAAAA,
  .data_length_code = 8,
  .data = {0, 1, 2, 3},
};



// can instatiation
CAN::CAN(gpio_num_t rxpin, gpio_num_t txpin){

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(txpin,rxpin,TWAI_MODE_NORMAL);

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    ESP_LOGI(TAG,"Driver installed\n");
  } else {
    ESP_LOGI(TAG,"Failed to install driver\n");
    return;
  }

  
  
}


void CAN::begin(){
  // Start TWAI driver
  if (twai_start() == ESP_OK) {
    ESP_LOGI(TAG,"Driver started\n");
  } else {
    ESP_LOGI(TAG,"Failed to start driver\n");
    return;
  }
  TimerHandle_t tx_timer = xTimerCreate("CANtx", pdMS_TO_TICKS(10), pdTRUE, NULL, txCallbackWrapper);

  xTimerStart(tx_timer,0);
  xTaskCreatePinnedToCore(rxTaskWrapper, "CAN_rx", 4096, this, 5, nullptr, 1);
}

void CAN::txCallbackWrapper(TimerHandle_t xTimer)
{
    CAN *instance = static_cast<CAN *>(static_cast<void *>(xTimer));
    instance->txCallback();
}

void CAN::txCallback(){
  //this function gets called every 10 milliseconds

  //10ms messages:

  if(txCallbackCounter%10 == 0){
    //100ms messages:
  }

  if(txCallbackCounter>=100){
    txCallbackCounter = 0;
  }
  txCallbackCounter++;
}

void CAN::rxTaskWrapper(void *arg)
{
    CAN *instance = static_cast<CAN *>(static_cast<void *>(arg));
    instance->rxTask();
}

void CAN::rxTask(){
  // create generic message
  twai_message_t rx_msg;
  ESP_LOGI(TAG,"Rx Task Started");
  while(1){
    // wait until a message is recieved, then send it to the queue.
    if(twai_receive(&rx_msg,portMAX_DELAY) == ESP_OK){

      uint32_t currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
      ESP_LOGI(TAG,"Message recieved with ID: %ld", rx_msg.identifier);

      if(rx_msg.identifier >= 1001 && rx_msg.identifier <= 1026){

        moboState.canTime = currentTime - moboState.canTime; //only update timeout for voltage and temp messages
        
        double s1 = (rx_msg.data[0] | (int)rx_msg.data[1] << 8)*0.001;
        double s2 = (rx_msg.data[2] | (int)rx_msg.data[3] << 8)*0.001;
        double s3 = (rx_msg.data[4] | (int)rx_msg.data[5] << 8)*0.001;
        double s4 = (rx_msg.data[6] | (int)rx_msg.data[7] << 8)*0.001;
        int id = rx_msg.identifier-1002;
        int module = id/5;
        int cell = id*4 - module*20;
        ESP_LOGI(TAG,"writing to module %d, id %d, starting cell %d", module, id, cell);
        moboState.setModuleVoltage(module,cell++,s1);
        moboState.setModuleVoltage(module,cell++,s2);
        moboState.setModuleVoltage(module,cell++,s3);
        moboState.setModuleVoltage(module,cell++,s4);
      }
      else if(rx_msg.identifier >= 1027 && rx_msg.identifier <= 1051){

        moboState.canTime = currentTime - moboState.canTime; //only update timeout for voltage and temp messages

        double s1 = (rx_msg.data[0] | (int)rx_msg.data[1] << 8)*0.01;
        double s2 = (rx_msg.data[2] | (int)rx_msg.data[3] << 8)*0.01;
        double s3 = (rx_msg.data[4] | (int)rx_msg.data[5] << 8)*0.01;
        double s4 = (rx_msg.data[6] | (int)rx_msg.data[7] << 8)*0.01;
        int id = rx_msg.identifier-1027;
        int module = id/5;
        int cell = id*4 - module*20;

        moboState.setModuleTemp(module,cell++,s1);
        moboState.setModuleTemp(module,cell++,s2);
        if((id+1)%5!= 0){
          moboState.setModuleTemp(module,cell++,s3);
          moboState.setModuleTemp(module,cell++,s4);
        }
      }
    }
  }
}
