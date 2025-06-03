#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "UARTto1Wire.h"
#include "LcdDisplay.h"

extern LcdDisplay lcd;

#define ONE_WIRE_BUS 25
static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature sensors(&oneWire);

#define UART1_TX_PIN 26
#define UART1_RX_PIN 27

void UARTto1Wire_setup() {
  Serial1.begin(globaluartbaudrate, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);

  sensors.begin();
  if (sensors.getDeviceCount() == 0) {
    Serial1.println("[UARTto1Wire] Không tìm thấy cảm biến DS18B20!");
  } else {
    Serial1.println("[UARTto1Wire] Cảm biến DS18B20 khởi động thành công.");
  }
  
  lcd.printStatus("UART", "1-Wire", globaluartbaudrate);
  delay(100);
}


void UARTto1Wire_loop() {
  static unsigned long lastReadTime = 0;
  const long readInterval = 500; // Đọc mỗi 500ms để không quá tải cảm biến
  
  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    Serial1.print("[UARTto1Wire] Nhiệt độ:");
    if (tempC != DEVICE_DISCONNECTED_C) {
      Serial1.print(tempC); // 2 chữ số thập phân
      Serial1.println("C");
    } else {
      Serial.println("Error:DS18B20_DISCONNECTED");
      Serial1.println("[UARTto1Wire] Lỗi: Cảm biến DS18B20 không kết nối.");
    
    }
  }
}