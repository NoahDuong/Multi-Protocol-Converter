#include "Bh1750Sensor.h"
#include <Arduino.h>

void Bh1750Sensor::selectChannel() {
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << BH1750_CHANNEL);
  Wire.endTransmission();
  delay(10);
}

Bh1750Sensor::Bh1750Sensor() {}

bool Bh1750Sensor::begin() {
  selectChannel();
  Wire.beginTransmission(BH1750_ADDR);
  if (Wire.endTransmission() == 0) {
    ready = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
    if (ready) {
      Serial.println("BH1750 ready at 0x23");
    }
  } else {
    Wire.beginTransmission(0x5C);
    if (Wire.endTransmission() == 0) {
      ready = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5C);
      if (ready) {
        Serial.println("BH1750 ready at 0x5C");
      }
    }
  }
  return ready;
}

float Bh1750Sensor::readLux(unsigned long &readTimeMs) {
  if (!ready) return -1;
  selectChannel();
  unsigned long start = micros();
  float lux = lightMeter.readLightLevel();
  readTimeMs = (micros() - start) / 1000;
  return lux;
}
