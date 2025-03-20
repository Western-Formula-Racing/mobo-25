#ifndef CONFIG_H
#define CONFIG_H
//digital inputs
#define BSPD_GPIO          GPIO_NUM_37   //BSPD Status Input pin
#define IMD_GPIO           GPIO_NUM_39   //IMD Status Input pin
#define LATCH_GPIO         GPIO_NUM_40   //Latch Status Input pin
#define ORION_GPIO         GPIO_NUM_5    //Orion Status (if orion is used)
#define HV_GPIO            GPIO_NUM_10   //HV Status (AIR+ on)
#define AIRN_GPIO          GPIO_NUM_11   //AIR- Status
#define CANTX              GPIO_NUM_47
#define CANRX              GPIO_NUM_48
//digital outputs
#define AMS_LATCH          GPIO_NUM_9    //AMS Latch output pin
#define PRECH_OK           GPIO_NUM_36   //Precharge OK output pin
#define FAN_PWM_PIN        GPIO_NUM_8    // Fan PWM pin
//analog inputs
#define CURSENSE_ADC       ADC_CHANNEL_1    //Current Sensor Input
#define PRECHSENSE_ADC     ADC_CHANNEL_0    //HV Precharge Sense Input
#define IMD_PWM            GPIO_NUM_12      //IMD Fault PWM Input

//BMS

#define BMS_CAN_TIMEOUT_ENABLED false
#define THRESHOLD_OVERVOLTAGE 4.3
#define THRESHOLD_UNDERVOLTAGE 2.7
#define THRESHOLD_OVERTEMP 60
#define TEMP_VOLTAGE_TIMEOUT_MS 1000
#define THRESHOLD_OVERCURRENT 100
#define THRESHOLD_CHARGECURRENT -6

#define MAX_FAN_POWER 0.5 // Fan power cap as a ratio, max 1
#define CURRENT_ADC_SCALING 9999999999
#define PRECHARGE_THRESHOLD 650000
#define PRECHARGE_OFFSET_MV 725

#endif