#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "stdio.h"

#define BMS_CAN_TIMEOUT_ENABLED false

#define THRESHOLD_OVERVOLTAGE 4.3
#define THRESHOLD_UNDERVOLTAGE 2.7
#define THRESHOLD_OVERTEMP 60
#define TEMP_VOLTAGE_TIMEOUT_MS 1000
#define THRESHOLD_OVERCURRENT 100
#define THRESHOLD_CHARGECURRENT -6



// Module information
typedef struct{
  int id;
  double voltage[20];
  double temp[18];
} Module_t;

typedef struct {

  bool overVoltage = 1;
  bool underVoltage = 1;
  bool overTemp = 1;
  bool CANTimeout = 1;
  bool overCurrent = 1;
  bool openCell = 1;
  bool openThermistor = 1;

}ErrorFlags;

//Motherboard System Status
class State{
  private:
    //things to keep track of
    double lowestVoltage; //lowest cell voltage
    double highestTemp; // highest thermistor temperature
    double SOC; // State of Charge
    bool chargeMode; // true means the accumulator is in charging mode

    //analog inputs to the mobo
    double packCurrent;           // Current measured from current sensor
    double prechargeVoltage;  // Precharge voltage sensed from flyback converter
    
    // safety loop - True means relay is closed, False means open. 
    //All of these are inputs with the exception of AMS, which is controlled by the motherboard
    bool imd;                //IMD status - True means relay is closed, False means open. 
    bool ams;                //AMS status - True means relay is closed, False means open. 
    bool bspd;               //BSPD status - True means relay is closed, False means open. 
    bool latch;              //Latch status - True means relay is closed, False means open. 
    bool precharge;          //Precharge status - True means precharge has begin, False means not yet. 
    bool HVact;              //AIR+ status - True = precharge is done and AIR+ is closed
    
    public:
    State();
    Module_t modules[5];     //array of modules
    ErrorFlags flags;
    uint16_t canTime;

    //getters and setters

    void setIMD(bool relayState);
    void setAMS(bool relayState);
    void setBSPD(bool relayState);
    void setLatch(bool relayState);
    void setPrecharge(bool relayState);
    void setHVAct(bool relayState);
    bool getIMD();
    bool getAMS();
    bool getBSPD();
    bool getLatch();
    bool getPrecharge();
    bool getHVAct();

    void setCurrent(double current);
    void setPrechargeVoltage(double voltage);
    double getCurrent();
    double getPrechargeVoltage();

    void setModuleVoltage(int module, int cell, double voltage);
    void setModuleTemp(int module, int thermistor, double temp);
    double getModuleVoltage(int module, int cell);
    double getModuleTemp(int module, int thermistor);

    double* getModuleVoltages(int module); //returns pointer to module voltage array
    double* getModuleTemps(int module); //returns pointer to module temp array
    double getAverageVoltage(); 
    double getAverageTemp();

    //functions
    void printModules(); // sends module info to serial
    void errorCheck();
    void resetFlags();
    void errorCheckWrapper(void *arg);
};

extern State moboState;

#endif