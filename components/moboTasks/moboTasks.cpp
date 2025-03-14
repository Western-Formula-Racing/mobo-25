#include "moboTasks.h"

//static const char *TAG = "Tasks";

//system variables

//things to keep track of
double lowestVoltage = 0.0; //lowest cell voltage
double highestTemp = 0.0;   // highest thermistor temperature
double SOC = 0;             // State of Charge
bool chargeMode;            // true means the accumulator is in charging mode

// safety loop - True means relay is closed, False means open. 
//All of these are inputs with the exception of AMS, which is controlled by the motherboard
bool imd = 0;                //IMD status - True means relay is closed, False means open. 
bool ams = 0;                //AMS status - True means relay is closed, False means open. 
bool bspd = 0;               //BSPD status - True means relay is closed, False means open. 
bool latch = 0;              //Latch status - True means relay is closed, False means open. 
bool precharge = 0;          //Precharge status - True means precharge has begin, False means not yet. 
bool HVact = 0;              //AIR+ status - True = precharge is done and AIR+ is closed

double prechargeVoltage = 0;

adc_oneshot_unit_handle_t adc1_handle;

void moboSetup(){
    // init inputs
    gpio_set_direction(BSPD_GPIO,GPIO_MODE_INPUT);
    gpio_set_direction(IMD_GPIO,GPIO_MODE_INPUT);
    gpio_set_direction(LATCH_GPIO,GPIO_MODE_INPUT);
    gpio_set_direction(ORION_GPIO,GPIO_MODE_INPUT);
    gpio_set_direction(HV_GPIO,GPIO_MODE_INPUT);
    gpio_set_direction(AIRN_GPIO,GPIO_MODE_INPUT);
    //init outputs
    gpio_set_direction(AMS_LATCH,GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(PRECH_OK,GPIO_MODE_OUTPUT);
  
    //setup ADC
    adc_oneshot_unit_init_cfg_t adcconfig = {
    .unit_id = ADC_UNIT_1,
    .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
  
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adcconfig, &adc1_handle));
  
    adc_oneshot_chan_cfg_t adc_chan_config = {
      .atten = ADC_ATTEN_DB_6,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
  
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle,PRECHSENSE_ADC, &adc_chan_config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle,CURSENSE_ADC, &adc_chan_config));  
}

void inputTask(void *pvParameters){
  while(1){
    //digital inputs
    imd = gpio_get_level(IMD_GPIO);
    ams = gpio_get_level(AMS_LATCH);
    bspd = gpio_get_level(BSPD_GPIO);
    latch = gpio_get_level(LATCH_GPIO);
    precharge = gpio_get_level(AIRN_GPIO);
    HVact = gpio_get_level(HV_GPIO);
    //analog (for now just raw values, later will be converted)
    int adc_raw = 0;
    adc_oneshot_read(adc1_handle,CURSENSE_ADC,&adc_raw);
    setCurrent(adc_raw*CURRENT_ADC_SCALING);
    adc_oneshot_read(adc1_handle,PRECHSENSE_ADC,&adc_raw);

  }
  
}