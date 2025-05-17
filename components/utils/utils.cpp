#include "utils.h"

adc_oneshot_unit_handle_t adc1_handle;

void setupADC(){

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
}

double getPrechargeVoltage(){
  //0v = 1.24V at the iso-opamp
  //24v = 1.33V out of the iso op-amp,
  //1V at HV = 0.00395833V at LV side
  //252.6315 V(HV)/V(LV)
  int raw;
  adc_oneshot_read(adc1_handle,PRECHSENSE_ADC,&raw);
  //printf("ADC raw reading: %d\n", raw);
  double voltage_mv = (double)raw / 4095.0 * 3.3;
  //printf("ADC Voltage reading: %.2f\n",voltage_mv);
  voltage_mv -= 1.19;
  voltage_mv *= 252.6315;
  //printf("Actual voltage: %.2f\n",voltage_mv);
  return voltage_mv;
}

double getSOC(){
  return 123.4;
}

double getPackCurrent(){
  int raw;
  adc_oneshot_read(adc1_handle,CURSENSE_ADC,&raw);
  //printf("ADC raw reading: %d\n", raw);
  double voltage_mv = (double)raw / 4095.0 * 3.3;
  voltage_mv -= 2.5;
  voltage_mv /= 4;
  return voltage_mv;
}

void printInfo(){
  printf(">State:%d\n",getStatus());
  printf(">IMD:%d|np\n",gpio_get_level(IMD_GPIO));
  printf(">AMS:%d|np\n",gpio_get_level(AMS_LATCH));
  printf(">LATCH:%d|np\n",gpio_get_level(LATCH_GPIO));
  printf(">AIR-:%d|np\n",gpio_get_level(AIRN_GPIO));
  printf(">HV:%d|np\n",gpio_get_level(HV_GPIO));
  printf(">Pack Current:%.3f\n",getPackCurrent());
  printf(">Pack Voltage: %.3f\n", getPackVoltage());
  printf(">Precharge Voltage:%.3f\n", getPrechargeVoltage());
  printf(">Error Number:%d\n",getErrorCode());
  printf(">Module 1 Timeout:%d\n",moduleTime[0]);
  printf(">Module 2 Timeout:%d\n",moduleTime[1]);
  printf(">Module 3 Timeout:%d\n",moduleTime[2]);
  printf(">Module 4 Timeout:%d\n",moduleTime[3]);
  printf(">Module 5 Timeout:%d\n",moduleTime[4]);
  if(gpio_get_level(GPIO_NUM_0) == 0){
    printModules();
  }

}