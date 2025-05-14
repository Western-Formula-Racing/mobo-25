#ifndef CONFIG_H
#define CONFIG_H
//digital inputs
#define BSPD_GPIO          GPIO_NUM_37   //BSPD Status Input pin
#define IMD_GPIO           GPIO_NUM_39   //IMD Status Input pin
#define LATCH_GPIO         GPIO_NUM_40   //Latch Status Input pin
#define HV_GPIO            GPIO_NUM_10   //HV Status (AIR+ on)
#define AIRN_GPIO          GPIO_NUM_11   //AIR- Status
#define CANTX              GPIO_NUM_47   //CANTX pin
#define CANRX              GPIO_NUM_48   //CANRX pin
#define CHARGE_PIN         GPIO_NUM_42   //if pulled down, accumulator is in chargecart
//digital outputs
#define AMS_LATCH          GPIO_NUM_9    //AMS Latch output pin
#define PRECH_OK           GPIO_NUM_36   //Precharge OK output pin
#define FAN_PWM_PIN        GPIO_NUM_8    // Fan PWM pin
//analog inputs
#define CURSENSE_ADC       ADC_CHANNEL_1    //Current Sensor Input
#define PRECHSENSE_ADC     ADC_CHANNEL_0    //HV Precharge Sense Input
#define ADC_SAMPLES        10
//5V IO pins
#define IO_1_5V            GPIO_NUM_5    //5VIO 1
#define IO_2_5V            GPIO_NUM_17   //5VIO 1
#define IO_3_5V            GPIO_NUM_18   //5VIO 1
#define IO_4_5V            GPIO_NUM_8    //5VIO 1

//BMS

#endif