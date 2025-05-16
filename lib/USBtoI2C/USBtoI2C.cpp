#include <Arduino.h>
#include "USBtoI2C.h"
#include "LcdDisplay.h"
#include "Bh1750Sensor.h"
#include "TCA9548A.h"


extern LcdDisplay lcd;
static Bh1750Sensor bh;
static uint32_t i2cFreq = 100000;

void USBtoI2C_setup() {
  Wire.begin(21, 22, i2cFreq);
  lcd.printStatus("USB", "I2C", i2cFreq);

  if (!bh.begin()) {
    Serial.println("Không tìm thấy BH1750!");
  } else {
    Serial.println("[USBtoI2C] BH1750 khởi động thành công.");
  }
}

void USBtoI2C_loop() {
  unsigned long readTimeMs = 0;
  float lux = bh.readLux(readTimeMs);
    // Hiển thị lên Serial Monitor
  Serial.print("[USBtoI2C] Lux: ");
  if (lux >= 0) {
    Serial.print(lux);
    Serial.println(" lx");
  } else {
    Serial.println("Error reading BH1750");
  }


  // Hiển thị lên LCD
  tcaselect(LCD_CHANNEL);
  lcd.setCursor(0, 0);
  lcd.print("IN:USB  OUT:I2C ");

  lcd.setCursor(0, 1);
  lcd.print("SP:");
  lcd.print(i2cFreq / 1000);
  lcd.print("kHz ");

  if (lux >= 0) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%4.1f", lux);
    lcd.print(buf);
    lcd.print("lx");
  } else {
    lcd.print(" Err ");
  }

  delay(1000);
}
