#include "BMS.h"
#include "CAN.cpp"

const char *TAG = "BMS";

Module modules[5];

void setModuleVoltage(int module, int cell, double newVoltage)
{
  if (module < 6 && cell < 21)
  {
    modules[module].voltage[cell] = newVoltage;
  }
  else
  {
    ESP_LOGE(TAG, "Voltage out of range!");
  }
}

void setModuleTemp(int module, int thermistor, double newTemp)
{
  if (module < 6 && thermistor < 19)
  {
    modules[module].temp[thermistor] = newTemp;
  }
  else
  {
    ESP_LOGE(TAG, "Temp out of range!");
  }
}

double getPackVoltage()
{
  double total = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 20; j++)
    {
      total += modules[i].voltage[j];
    }
  }
  return total;
}

double getMaxTemp()
{
  double max = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 18; j++)
    {
      if (max < modules[i].temp[j])
      {
        max = modules[i].temp[j];
      }
    }
  }
  return max;
}

double getMaxTemp(int &index)
{
  double max = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 18; j++)
    {
      if (max < modules[i].temp[j])
      {
        max = modules[i].temp[j];
        index = i * 18 + j;
      }
    }
  }
  return max;
}

double getMaxVoltage()
{
  double max = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 20; j++)
    {
      if (max < modules[i].voltage[j])
      {
        max = modules[i].voltage[j];
      }
    }
  }
  return max;
}

double getMaxVoltage(int &index)
{
  double max = 0;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 20; j++)
    {
      if (max < modules[i].voltage[j])
      {
        max = modules[i].voltage[j];
        index = i * 20 + j;
      }
    }
  }
  return max;
}

double getMinVoltage()
{
  double min = 100;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 20; j++)
    {
      if (min > modules[i].voltage[j])
        min = modules[i].voltage[j];
    }
  }
  return min;
}

double getMinVoltage(int &index)
{
  double min = 100;
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 20; j++)
    {
      if (min > modules[i].voltage[j])
      {
        min = modules[i].voltage[j];
        index = i * 20 + j;
      }
    }
  }
  return min;
}

// serial debugging
void printModules()
{
  printf("===Module Info===\n");
  printf("   Module 1 --- Cells        Module 2 --- Cells        Module 3 --- Cells        Module 4 --- Cells        Module 5 --- Cells     \n");
  for (int k = 0; k < 5; k++)
  {
    for (int j = 0; j < 5; j++)
    {
      for (int i = 0; i < 4; i++)
      {
        printf("|%.3f", modules[j].voltage[i + k * 4]);
      }
      printf("| ");
    }
    printf("\n");
  }
  printf("   Module 1 --- Temp        Module 2 --- Temp        Module 3 --- Temp        Module 4 --- Temp        Module 5 --- Temp     \n");
  for (int k = 0; k < 5; k++)
  {
    for (int j = 0; j < 5; j++)
    {
      for (int i = 0; i < 4; i++)
      {
        if (i + k * 4 < 18)
        {
          printf("|%.3f", modules[j].temp[i + k * 4]);
        }
        else
        {
          printf("|x.xxx");
        }
      }
      printf("| ");
    }
    printf("\n");
  }
}

// SoC Code (Coulomb counting)

double setHallCalibration()
{
  float hallZero;
  if (txCounter >= 100)
  {
    if (inverterCurrent() == 0)
    {
      hallZero = CURSENSE_ADC;
      txCounter = 0;
    }
  }
  txCounter++;
  return hallZero;
  // if inverter DC current in = 0 & inverter status message is active; then hallZero is equal to whatever CURSENSE_ADC is; if inverter is on just give the last value
}

double getHallCurrent()
{
  double hallCurrent;
  if (txCounter > 100)
  {
    if ((CURSENSE_ADC - setHallCalibration()) >= 0)
    {
      hallCurrent = ((10 / 15.1) * (CURSENSE_ADC - setHallCalibration()) / (5.7 * 1e-3));
    }
    else
    {
      hallCurrent = 0;
      ESP_LOGE(TAG, "currentCharge sensor is improperly calibarated (negative current)");
    }

    txCounter = 0;
  }
  txCounter++;
  return hallCurrent;
}

// double setHallCalibrationElCon() { *****You are not using the current sensor when charging*****
//   float hallZero;
//   if (elconCurrent()==0){
//     hallZero = CURSENSE_ADC;
//   }
// return hallZero;

// //When ElCon current ouput = 0 & ElCon status message is active; then hallZero is equal to whatever CURSENSE_ADC is; if ElCon is on just give the last value

// }

double getMaxCharge()
{
  // if getPackVoltage() is 300-305 V and ElCon output current is greater then zero then enter a loop that adds to a variable, when the ElCon output goes to zero exit the loop and decide wether to
  // add the total charge to just currentCharge or both currentCharge and MaxCharge based on if we charged the back up to 405-410V (set them equal and return one of them)
  double maxCharge;
  double currentCharge;
  if ((getStatus() == CHARGING) && (getPackVoltage() > 300 && getPackVoltage() < 305) && (elconCurrent() > 0))
  {
    if (txCounter >= 100)
    {
      if (getPackVoltage() < 400)
      {
        currentCharge += elconCurrent();
      }
      else if (getPackVoltage() > 400 && getPackVoltage() < 405)
      {
        maxCharge = currentCharge;
      }
      txCounter = 0;
    }
    txCounter++;
  }
  return maxCharge;
}

double getCurrentCharge()
{
  double currentCharge;
  // loop through calling your current flow function every 100ms or so and subtract it from whatever your current charge; you must find a way to return the currentCharge inn the getMaxCharge was initially
  if (txCounter >= 100)
  {
    if ((getStatus() == CHARGING) && (CHARGE_CURRENT > 0))
    { // wrong variable `CHARGE_CURRENT`

      currentCharge += CHARGE_CURRENT;
      txCounter = 0;
    }
  }
  else if ((getStatus() == ACTIVE || getStatus() == PRECHARGE_START || getStatus() == PRECHARGE_CHECK))
  {
    currentCharge -= getHallCurrent();
    txCounter = 0;
  }
  txCounter++;
  return currentCharge;
}

double getSoC()
{
  double SoC;

  if (txCounter >= 100)
  {
    SoC = getCurrentCharge() / getMaxCharge() * 100;
    txCounter = 0;
  }
  txCounter++;
  return SoC;
}

double inverterCurrent()
{
  double dcBusCurrent;
  if (txCounter >= 10)
  {

    twai_message_t msg;

    if (msg.identifier == 0x8D001031 && msg.data_length_code == 8)
    {                                                        // identifies the correct CAN message before decoding
      int16_t rawCurrent = msg.data[6] | (msg.data[7] << 8); // extracts the 16 bit signed int from the last two bytes of the CAN message payload;
      double dcBusCurrent = rawCurrent * 0.1;                // converts value to amps
    }

    txCounter = 0;
  }
  txCounter++;
  return dcBusCurrent;
}

double elconCurrent()
{
  //   double chargeCurrent;
  // if(txCounter>=10){

  //   twai_message_t msg;

  //   if (msg.identifier == 0x8D001031 && msg.data_length_code ==8){ //identifies the correct CAN message before decoding
  //     int16_t rawCurrent = msg.data[6] | (msg.data [7]<<8); //extracts the 16 bit signed int from the last two bytes of the CAN message payload;
  //     double dcBusCurrent = rawCurrent * 0.1; //converts value to amps
  //   }

  //   txCounter = 0;
  //         }
  //     txCounter++;
  //     return dcBusCurrent;
}
