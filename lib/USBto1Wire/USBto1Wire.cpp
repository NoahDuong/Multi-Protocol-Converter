#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "USBto1Wire.h"
#include "LcdDisplay.h"

extern LcdDisplay lcd;
#define ONE_WIRE_BUS 25
static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature sensors(&oneWire);

void USBto1Wire_setup() {
  sensors.begin();
  lcd.printStatus("USB", "1-Wire", 16300);  // 16.3 kbps (mặc định)
  Serial.println("[USBto1Wire] Cảm biến DS18B20 khởi động.");
}

void USBto1Wire_loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  // In ra Serial Monitor
  Serial.print("[USBto1Wire] Nhiệt độ: ");
  if (tempC != DEVICE_DISCONNECTED_C) {
    Serial.print(tempC);
    Serial.println(" °C");
  } else {
    Serial.println("Thiết bị không kết nối!");
  }

  // lcd
  lcd.setCursor(0, 0);
  lcd.print("IN:USB OUT:1-Wire");

  lcd.setCursor(0, 1);
  lcd.print("SP:16kHz ");

  if (tempC != DEVICE_DISCONNECTED_C) {
    char buf[10];
    dtostrf(tempC, 5, 2, buf);  // width 5, 2 số thập phân
    lcd.print(buf);
    lcd.write(0xDF);            // in ký tự độ (°)
    lcd.print("C");
  } else {
    lcd.print(" Err ");
  }

  delay(1000);
}
