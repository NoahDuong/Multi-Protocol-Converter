#include <Arduino.h>
#include "USBtoI2C.h"
#include "LcdDisplay.h"
#include "Bh1750Sensor.h"
#include "TCA9548A.h"


extern LcdDisplay lcd;
static Bh1750Sensor bh;

void USBtoI2C_setup() {
  lcd.printStatus("USB", "I2C", globali2cFrequency);

  if (!bh.begin()) {
    Serial.println("Không tìm thấy BH1750!");
  } else {
    Serial.println("[USBtoI2C] BH1750 khởi động thành công.");
  }
}

void USBtoI2C_loop() {
  static unsigned long lastreadtime = 0;
  unsigned long readinterval = 1000;

  if (globali2cFrequency == 100000) {
    readinterval = 10000;
  } else if (globali2cFrequency == 400000) {
    readinterval = 4000;
  } else if (globali2cFrequency == 1000000) {
    readinterval = 1000;
  }


  if (millis() - lastreadtime >= readinterval) {
    lastreadtime = millis();
    unsigned long readTimeMs = 0;
    float lux = bh.readLux(readTimeMs); // Đọc lux từ cảm biến BH1750

    Serial.print("[USBtoI2C] Freq: ");
    Serial.print(globali2cFrequency / 1000);
    Serial.print("kHz. Interval: ");
    Serial.print(readinterval/1000);
    Serial.print("s. Lux: ");
    if (lux >= 0) {
      Serial.print(lux);
      Serial.println(" lx");
    } else {
      Serial.println("Error reading BH1750");
    }
  }

  tcaselect(LCD_CHANNEL);
  // lcd.setCursor(0, 0);
  // lcd.print("IN:USB  OUT:I2C ");
  // lcd.setCursor(0, 1);
  // lcd.print("SP:");
  // lcd.print(globali2cFrequency / 1000);
  // lcd.print("kHz ");
  // if (lux >= 0) {
  //   char buf[6];
  //   snprintf(buf, sizeof(buf), "%4.1f", lux);
  //   lcd.print(buf);
  //   lcd.print("lx");
  // } else {
  //   lcd.print("Error");
  // }
  delay(10);
}
