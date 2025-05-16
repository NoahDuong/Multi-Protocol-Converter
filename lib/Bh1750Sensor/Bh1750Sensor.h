#pragma once
#include <Wire.h>
#include <BH1750.h>

#define TCAADDR 0x70
#define BH1750_CHANNEL 1
#define BH1750_ADDR 0x23

class Bh1750Sensor {
private:
  BH1750 lightMeter;
  bool ready = false;

  void selectChannel();

public:
  Bh1750Sensor();
  bool begin();
  float readLux(unsigned long &readTimeMs);
};
