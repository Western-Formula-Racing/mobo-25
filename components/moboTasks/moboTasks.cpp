#include "moboTasks.h"

static const char *TAG = "Tasks";

//system variables

//things to keep track of
double lowestVoltage = 0.0; //lowest cell voltage
double highestTemp = 0.0;   // highest thermistor temperature
double SOC = 0;             // State of Charge
state status = IDLE;

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
adc_cali_handle_t adc_cali_handle;

void moboSetup(){

    // init inputs
    gpio_set_direction(BSPD_GPIO,GPIO_MODE_INPUT);
    gpio_set_direction(IMD_GPIO,GPIO_MODE_INPUT);
    gpio_set_direction(LATCH_GPIO,GPIO_MODE_INPUT);
    gpio_set_direction(ORION_GPIO,GPIO_MODE_INPUT);
    gpio_set_direction(HV_GPIO,GPIO_MODE_INPUT);
    gpio_set_direction(AIRN_GPIO,GPIO_MODE_INPUT);
    gpio_set_direction(CHARGE_PIN,GPIO_MODE_INPUT);

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
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle,PRECHSENSE_ADC, &adc_chan_config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle,CURSENSE_ADC, &adc_chan_config));  
    
    //setup ADC Curve fitting scheme
    adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = ADC_UNIT_1,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle));
    
    //close AMS relay
    gpio_set_level(AMS_LATCH,1);
    //Setup Fan control PWM

    ledc_timer_config_t ledc_timer = {
      .speed_mode       = LEDC_LOW_SPEED_MODE,
      .duty_resolution  = LEDC_TIMER_8_BIT,
      .timer_num        = LEDC_TIMER_0,
      .freq_hz          = 25000,
      .clk_cfg          = LEDC_AUTO_CLK
    };

    ledc_channel_config_t ledc_channel = {
      .gpio_num       = FAN_PWM_PIN,
      .speed_mode     = LEDC_LOW_SPEED_MODE,
      .channel        = LEDC_CHANNEL_0,
      .intr_type      = LEDC_INTR_DISABLE,
      .timer_sel      = LEDC_TIMER_0,
      .duty           = 0, // Set duty to 0%
      .hpoint         = 0,
    };
  
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void inputTask(void *pvParameters){
  int adc_raw = 0;
  int adc_reading = 0;
  int adc_voltage = 0;
  double current = 0;
  while(1){
    //digital inputs
    imd = gpio_get_level(IMD_GPIO);
    ams = gpio_get_level(AMS_LATCH);
    bspd = gpio_get_level(BSPD_GPIO);
    latch = gpio_get_level(LATCH_GPIO);
    precharge = gpio_get_level(AIRN_GPIO);
    HVact = gpio_get_level(HV_GPIO);

    //analog inputs
    //Multisampling
    adc_oneshot_read(adc1_handle,CURSENSE_ADC,&adc_raw);
    printf(">current_adc_raw:%d\n",adc_raw);
    adc_cali_raw_to_voltage(adc_cali_handle,adc_raw,&adc_voltage);
    //divide by 0.6 for voltage divider to get back to 5v resolution, then subtract 2.5v and divide by 66.7 mv/A
    printf(">current_adc_voltage:%d\n",adc_voltage);
    current = (double)adc_voltage/0.6;
    current -= 2500.0;
    current /= 5.7;
    current -= 5.26;
    setCurrent(current);

    //read precharge voltage - Conversion is ~2.4mV/V, so at 60V we expect 133mV. The iso-opamp has an offset of ~725mV
    //this is honestly a pretty poor estimate of the actual voltage, but it works for our purposes
    adc_oneshot_read(adc1_handle,PRECHSENSE_ADC,&adc_raw);
    adc_cali_raw_to_voltage(adc_cali_handle,adc_raw,&adc_voltage); //get adc reading in mV
    adc_voltage -= PRECHARGE_OFFSET_MV; //apply offset
    adc_voltage *= 416.6;
    prechargeVoltage = adc_voltage;
    
    if(gpio_get_level(CHARGE_PIN) == 0 && getStatus() == ACTIVE){
      setStatus(CHARGING);
    }
    else if(getStatus()==CHARGING && gpio_get_level(CHARGE_PIN) == 1){
      setStatus(ACTIVE);
      elconControl(0,0,0);
    }

    //printf("Precharge voltage: %.2f", prechargeVoltage);
    vTaskDelay(pdMS_TO_TICKS(100)); //run every 100ms
    
  }
}

void prechargeTask(void* pvParameters){
  int prechargeCounter = 0;
  while(1){
    if(status==IDLE && precharge == true){
      status = PRECHARGE;
    }
    if(status == PRECHARGE){

      if(prechargeVoltage>(getPackVoltage()*0.9) && prechargeCounter > 3){
        gpio_set_level(PRECH_OK,1); // close AIR+
        status = ACTIVE;
      } 
      else if(precharge == 1 && prechargeVoltage>(getPackVoltage()*0.9)){
        prechargeCounter++;
        vTaskDelay(pdMS_TO_TICKS(50));
      }
      else{
        gpio_set_level(PRECH_OK,0); // open AIR+
        prechargeCounter = 0;
      }
    }

    if(status == CHARGING){
      elconControl(MAX_CHARGE*100,5.0,1);
      if(getMaxVoltage()>MAX_CHARGE){

        ESP_LOGI(TAG,"Charging complete; Waiting 10 seconds...");
        for(int i = 0; i<10;i++){
          ESP_LOGI(TAG,"%d seconds...",i);
          vTaskDelay(pdMS_TO_TICKS(1000));
        }
        ESP_LOGE(TAG,"Lowest Voltage: %.3f",getMinVoltage());
      }
    }
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void coolingTask(void *pvParameters){
  double temp;
  int dutyCycle;
  while(1){
    temp = getMaxTemp();
    //calculate fan power from temps:
    if(temp>=THRESHOLD_OVERTEMP-5){
      dutyCycle = 255; //emergency cooling mode when within 5 degrees of maximum
    }
    else{
      dutyCycle = (255*MAX_FAN_POWER)*(getMaxTemp()/THRESHOLD_OVERTEMP);
    }

    ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0,dutyCycle);
    ledc_update_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0);

    vTaskDelay(1000); //update once a second
  }
}

void setStatus(state newStatus){
  status = newStatus;
}

state getStatus(){
  return status;
}

//print out telemetry for teleplot
void telemetryTask(void *pvParameters){
  while(1){

    printf(">IMD Relay:%d|np\n",imd);
    printf(">AMS Relay:%d|np\n",ams);
    printf(">BSPD Relay:%d|np\n",bspd);
    printf(">Latch Relay:%d|np\n",latch);
    printf(">Sloop Return:%d|np\n",precharge);
    printf(">HV Active:%d|np\n",HVact);
    printf(">Status:%d|np\n",status);
    printf(">Error:%d|np\n",getErrorFlags().errorCode);
    printf(">Precharge_Voltage:%.2f\n",prechargeVoltage);
    printf(">Pack_Current:%.2f\n",getPackCurrent());
    printf(">Pack_voltage:%.2f\n",getPackVoltage());
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}