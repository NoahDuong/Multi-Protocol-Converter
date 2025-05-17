#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "UARTto1Wire.h"
#include "LcdDisplay.h"

extern LcdDisplay lcd;

#define ONE_WIRE_BUS 25
static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature sensors(&oneWire);

#define UART1_TX_PIN 26 // Chân TX của UART1
#define UART1_RX_PIN 27 // Chân RX của UART1
static uint32_t uartBaudRate = 115200; // Tốc độ baud cho UART1


void UARTto1Wire_setup() {
  // Khởi tạo UART1 nếu chưa được khởi tạo
  Serial1.begin(uartBaudRate, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);
  Serial.println("[UARTto1Wire] Khởi động UART1 thành công.");

  // Khởi tạo cảm biến 1-Wire
  sensors.begin();
  if (sensors.getDeviceCount() == 0) {
    Serial.println("[UARTto1Wire] Không tìm thấy cảm biến DS18B20!");
  } else {
    Serial.println("[UARTto1Wire] Cảm biến DS18B20 khởi động thành công.");
  }
  
  // Cập nhật trạng thái trên LCD
  lcd.printStatus("UART", "1-Wire", uartBaudRate);
  delay(100);
}


void UARTto1Wire_loop() {
  static unsigned long lastReadTime = 0;
  const long readInterval = 500; // Đọc mỗi 500ms để không quá tải cảm biến

  if (Serial1.available()) {
    char command = Serial1.read();
    if (command == 'T') { // Yêu cầu đọc nhiệt độ
      sensors.requestTemperatures();
      float tempC = sensors.getTempCByIndex(0);

      // Gửi kết quả trở lại qua UART1
      Serial1.print("Temp:");
      if (tempC != DEVICE_DISCONNECTED_C) {
        Serial1.print(tempC); // 2 chữ số thập phân
        Serial1.println("C");
        Serial.printf("[UARTto1Wire] Gửi nhiệt độ: %.2fC qua UART1\n", tempC);
        Serial.print("[UARTto1Wire] Nhiệt độ (qua UART1): ");
      } else {
        Serial1.println("Error:DS18B20_DISCONNECTED");
        Serial.println("[UARTto1Wire] Lỗi: Cảm biến DS18B20 không kết nối.");
      }
    }
  }

  // Cập nhật LCD thường xuyên để hiển thị trạng thái
  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    lcd.setCursor(0, 0);
    lcd.print("IN:UART  OUT:1-W");
    
    lcd.setCursor(0, 1);
    lcd.print("SP:"); lcd.print(uartBaudRate / 1000); lcd.print("k ");

    if (tempC != DEVICE_DISCONNECTED_C) {
      lcd.print("T:"); lcd.print(tempC, 1); lcd.write(0xDF); lcd.print("C");
    } else {
      lcd.print("Sens Disconnect");
    }
  }
}