#include "CAN.h"
static const char *TAG = "CAN";

bool module_timeout[] = {false, false, false, false, false};
int64_t last_received[] = {0, 0, 0, 0, 0};

uint32_t alerts_to_enable = TWAI_ALERT_ABOVE_ERR_WARN|TWAI_ALERT_ERR_ACTIVE|TWAI_ALERT_RX_QUEUE_FULL;

twai_message_t txMessage = {

  // Message type and format settings
  .extd = 1,              // Standard vs extended format
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
CAN::CAN(gpio_num_t rxpin, gpio_num_t txpin){

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(txpin,rxpin,TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  
  ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
  
  //if on charger, send charge status to BMS then switch to 250kbps
  if (gpio_get_level(CHARGE_PIN) == 0){
    txMessage.identifier = 1056;
    txMessage.data[0] = 0;
    txMessage.data[1] = 0;
    txMessage.data[2] = 0;
    txMessage.data[3] = 0;
    txMessage.data[4] = 0;
    txMessage.data[5] = 3; // charging status bit 
    txMessage.data[6] = 0;
    txMessage.data[7] = 0;
    twai_transmit(&txMessage,portMAX_DELAY);
    twai_driver_uninstall();
    ESP_LOGI(TAG,"Entering Charge Mode...");
    t_config = TWAI_TIMING_CONFIG_250KBITS();
    ESP_ERROR_CHECK(twai_driver_install(&g_config,&t_config,&f_config));
  }

  ESP_ERROR_CHECK(twai_reconfigure_alerts(alerts_to_enable,NULL));

  
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
  xTaskCreatePinnedToCore(timeoutHandler, "timeoutHandler", 4096, this, 5, nullptr, 1);
}

void CAN::timeoutHandler(void* arg)
{
  for(;;){
    for(uint8_t i = 0; i<= 4; i++){
      int64_t current_time = xTaskGetTickCount() * (1000/configTICK_RATE_HZ);
      printf(">current_time:%lld\n", current_time);
      printf(">last_received[%d]:%lld\n", i, last_received[i]);
      printf(">(current_time - last_received[i]):%lld\n", (current_time - last_received[i]));
      if((current_time - last_received[i]) >= CAN_TIMEOUT ){
        errorFlags error = {true, i, 83};
        setErrorFlags(error);
        printf("timeout error on module %d\n", i);
        // raiseError();

      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
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
    txMessage.identifier = 1056;
    int16_t current = (int16_t)getPackCurrent()*10;
    txMessage.data[0] = (uint8_t)(current & 0x0F);
    txMessage.data[1] = (uint8_t)(current & 0xF0)>>8;
    bool imd = gpio_get_level(IMD_GPIO);
    bool ams = gpio_get_level(AMS_LATCH);
    bool bspd = gpio_get_level(BSPD_GPIO);
    bool latch = gpio_get_level(LATCH_GPIO);
    bool prech_en = gpio_get_level(AIRN_GPIO);
    bool HVAct = gpio_get_level(HV_GPIO);
    txMessage.data[2] = (uint8_t)(imd & (ams<<1) & (bspd<<2) & (latch<<3) & (prech_en<<4) & (HVAct<<5));
    uint16_t SOC = (uint16_t)(getSOC() * 100);
    txMessage.data[3] = (uint8_t)SOC & 0x0F;
    txMessage.data[4] = (uint8_t)(SOC & 0xF0)>>8;
    txMessage.data[5] = getStatus();
    txMessage.data[5] = getErrorFlags().errorCode;

    twai_transmit(&txMessage,portMAX_DELAY);
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
    // wait until a message is recieved (up to 2 seconds).
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
        //update last received for timeout
        last_received[module] = xTaskGetTickCount() * (1000/configTICK_RATE_HZ); // time in milliseconds
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
        last_received[module] = xTaskGetTickCount() * (1000/configTICK_RATE_HZ); // time in milliseconds
      }
      else if(rx_msg.identifier == 1000){
        //Error message!
        uint8_t module_id = rx_msg.data[0];
        uint8_t errorcode = rx_msg.data[1];
        double voltage = (rx_msg.data[2] | (int)rx_msg.data[3] << 8)*0.001;
        uint8_t voltageIndex = rx_msg.data[4];
        double temperature = (rx_msg.data[2] | (int)rx_msg.data[3] << 8)*0.01;
        uint8_t tempIndex = rx_msg.data[7];
        errorFlags newError = {
          .errored = true,
          .moduleNumber = module_id,
          .errorCode = errorcode,
          .cellVoltage = voltage,
          .cellIndex = voltageIndex,
          .thermistorTemp = temperature,
          .thermistorIndex = tempIndex
        };
        setErrorFlags(newError);
      }
    }
  }
}

void canAlertTask(void *pvParameters){
  uint32_t alerts_triggered;
  errorFlags newError;
  while(1){
    twai_read_alerts(&alerts_triggered, portMAX_DELAY);
    if(alerts_triggered!=TWAI_ALERT_NONE){
      newError= {
        .errored = true,
        .errorCode = 85
      };
      // setErrorFlags(newError);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}