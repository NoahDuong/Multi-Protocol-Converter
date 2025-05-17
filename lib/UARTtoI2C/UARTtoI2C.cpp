#include <Arduino.h>
#include <Wire.h>
#include "UARTtoI2C.h"
#include "LcdDisplay.h"
#include "Bh1750Sensor.h"
#include "TCA9548A.h" // Để sử dụng tcaselect

// Biến extern cho LCD (được định nghĩa trong main.cpp)
extern LcdDisplay lcd;
static Bh1750Sensor bh; // Đối tượng cảm biến BH1750

// Cấu hình UART
#define UART1_TX_PIN 26 // Chân TX của UART1
#define UART1_RX_PIN 27 // Chân RX của UART1
static uint32_t uartBaudRate = 115200; // Tốc độ baud cho UART1

// Cấu hình I2C (thường dùng chân SDA=21, SCL=22)
static uint32_t i2cFreq = 100000; // 100 kHz


void UARTtoI2C_setup() {
  // Khởi tạo UART1 nếu chưa được khởi tạo
  Serial1.begin(uartBaudRate, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);
  Serial.println("[UARTtoI2C] Khởi động UART1 thành công.");

  // Khởi tạo giao tiếp Wire (I2C)
  // Đảm bảo Wire.begin đã được gọi trong setup() của main hoặc gọi lại ở đây
  Wire.begin(21, 22, i2cFreq); // SDA, SCL, tần số
  Wire.setClock(i2cFreq);
  Serial.printf("[UARTtoI2C] Khởi tạo I2C tần số %luHz\n", i2cFreq);

  // Khởi tạo cảm biến BH1750
  if (!bh.begin()) {
    Serial.println("[UARTtoI2C] Không tìm thấy BH1750!");
  } else {
    Serial.println("[UARTtoI2C] BH1750 khởi động thành công.");
  }

  // Cập nhật trạng thái trên LCD
  lcd.printStatus("UART", "I2C", uartBaudRate); // Hiển thị tốc độ UART
  delay(100);
}


void UARTtoI2C_loop() {
  static unsigned long lastReadTime = 0;
  const long readInterval = 500; // Đọc mỗi 500ms để không quá tải cảm biến

  if (Serial1.available()) {
    char command = Serial1.read();
    if (command == 'L') { // Yêu cầu đọc lux
      unsigned long readTimeMs = 0;
      float lux = bh.readLux(readTimeMs);

      // Gửi kết quả trở lại qua UART1
      if (lux >= 0) {
        Serial1.print("Lux:");
        Serial1.print(lux, 2); // 2 chữ số thập phân
        Serial1.println("lx");
        Serial.printf("[UARTtoI2C] Gửi Lux: %.2flx qua UART1\n", lux);
      } else {
        Serial1.println("Error:BH1750_READ_FAIL");
        Serial.println("[UARTtoI2C] Lỗi: Không đọc được BH1750.");
      }
    }
  }

  // Cập nhật LCD thường xuyên để hiển thị trạng thái
  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();
    unsigned long readTimeMs = 0;
    float lux = bh.readLux(readTimeMs);

    lcd.setCursor(0, 0);
    lcd.print("IN:UART  OUT:I2C");
    
    lcd.setCursor(0, 1);
    lcd.print("SP:"); lcd.print(uartBaudRate / 1000); lcd.print("k ");

    if (lux >= 0) {
      lcd.print(lux, 2); lcd.print("Lx");
    } else {
      lcd.print("Error");
    }
  }
}