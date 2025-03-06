#include "state.h"
#include "esp_log.h"

static const char *TAG = "State";

State moboState = State();

State::State(){

  imd = ams = bspd = latch = precharge = HVact = lowestVoltage = highestTemp = chargeMode = 0;

};

//setters

void State::setIMD(bool relayState){
  imd = relayState;
  ESP_LOGD(TAG,"Set IMD to %d",imd);
};
void State::setAMS(bool relayState){
  ams = relayState;
  ESP_LOGD(TAG,"Set AMS to %d",ams);
};
void State::setBSPD(bool relayState){
  bspd = relayState;
  ESP_LOGD(TAG,"Set BSPD to %d",bspd);
};
void State::setLatch(bool relayState){
  latch = relayState;
  ESP_LOGD(TAG,"Set Latch to %d", latch);
};
void State::setPrecharge(bool relayState){
  precharge = relayState;
  ESP_LOGD(TAG,"Set Precharge to %d", precharge);
};
void State::setHVAct(bool relayState){
  HVact = relayState;
  ESP_LOGD(TAG,"Set HVact to %d", HVact);
};
void State::setCurrent(double current){
  packCurrent = current;
  ESP_LOGD(TAG,"Set current to %.3f", packCurrent);
};
void State::setPrechargeVoltage(double voltage){
  prechargeVoltage = voltage;
  ESP_LOGD(TAG,"Set precharge voltage to %.3f", prechargeVoltage);
};
void State::setModuleVoltage(int module, int cell, double voltage){
  modules[module].voltage[cell] = voltage;
  ESP_LOGD(TAG, "Set module %d, cell %d to %.3f", module,cell,modules[module].voltage[cell]);
  if(voltage<lowestVoltage){
    lowestVoltage = voltage;
  }
};
void State::setModuleTemp(int module, int thermistor, double temp){
  modules[module].temp[thermistor] = temp;
  ESP_LOGD(TAG, "Set module %d, thermistor %d to %.3f", module,thermistor,modules[module].temp[thermistor]);
  if(temp>highestTemp){
    highestTemp = temp;
  }
};

//getters

bool State::getIMD(){
  return imd;
};
bool State::getAMS(){
  return ams;
};
bool State::getBSPD(){
  return bspd;
};
bool State::getLatch(){
  return latch;
};
bool State::getPrecharge(){
  return precharge;
};
bool State::getHVAct(){
  return HVact;
};

double State::getModuleVoltage(int module, int cell){
  return modules[module].voltage[cell];
};
double State::getModuleTemp(int module, int thermistor){
  return modules[module].temp[thermistor];
};

double* State::getModuleVoltages(int module){
  return modules[module].voltage;
}; //returns pointer to module voltage array
double* State::getModuleTemps(int module){
  return modules[module].temp;
}; //returns pointer to module temp array

double State::getAverageVoltage(){
  double sum;
  for(int i =0; i<5;i++){
    for(int j=0;j<20;j++){
      sum += modules[i].voltage[j];
    }
  }
  return sum/100;
}; 
double State::getAverageTemp(){
  double sum;
  for(int i =0; i<5;i++){
    for(int j=0;j<18;j++){
      sum += modules[i].temp[j];
    }
  }
  return sum/100;
};