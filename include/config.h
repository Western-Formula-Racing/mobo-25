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
#define STATUS_LED         GPIO_NUM_16   // Green LED
#define FAULT_LED          GPIO_NUM_15   // Red LED
#define TSSI_ENABLE        GPIO_NUM_4    // TSSI Enable pin
//analog inputs
#define CURSENSE_ADC       ADC_CHANNEL_1    //Current Sensor Input
#define PRECHSENSE_ADC     ADC_CHANNEL_0    //HV Precharge Sense Input
#define ADC_SAMPLES        4
//5V IO pins
#define IO_1_5V            GPIO_NUM_5    //5VIO 1
#define IO_2_5V            GPIO_NUM_17   //5VIO 1
#define IO_3_5V            GPIO_NUM_18   //5VIO 1
#define IO_4_5V            GPIO_NUM_8    //5VIO 1

//BMS
#define MAX_CHARGE         4.1    //max cell voltage to charge to
#define CHARGE_CURRENT     5      //max charge current in amps 

//CAN
#define CAN_TIMEOUT 100000 // Allowable CAN Errors

//Error thresholds
#define THRESHOLD_OVERTEMP      60
#define THRESHOLD_OVERVOLTAGE   4.15
#define THRESHOLD_UNDERVOLTAGE  2.6   
#define THRESHOLD_MAXDELTA      0.2
#define THRESHOLD_OVERCURRENT   150
#define THRESHOLD_CANTIMEOUT    20000 //milliseconds

//telemetry enable
#define TELEMETRY true

#endif