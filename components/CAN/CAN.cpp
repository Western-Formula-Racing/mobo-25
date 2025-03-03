#include "CAN.h"
static const char *TAG = "CAN";

QueueHandle_t can_queue;

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
  ESP_LOGI(TAG,"10ms message");


  if(txCallbackCounter%10 == 0){
    //100ms messages:
    ESP_LOGI(TAG,"100ms message");
  }

  if(txCallbackCounter>=100){
    ESP_LOGI(TAG,"1 second message");
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

  while(1){
    // wait until a message is recieved, then send it to the queue.
    if(twai_receive(&rx_msg,portMAX_DELAY) == ESP_OK){
      xQueueSend(can_queue, &rx_msg, pdMS_TO_TICKS(10));
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}