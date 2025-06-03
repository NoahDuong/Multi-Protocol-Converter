#include <Arduino.h>
#include <Wire.h>
#include "UARTtoI2C.h"
#include "LcdDisplay.h"
#include "Bh1750Sensor.h"
#include "TCA9548A.h" // Để sử dụng tcaselect

extern LcdDisplay lcd;
static Bh1750Sensor bh;

#define UART1_TX_PIN 26
#define UART1_RX_PIN 27

void UARTtoI2C_setup() {
  Serial1.begin(globaluartbaudrate, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);
  Serial1.printf("[UARTtoI2C] Khởi động UART1 thành công vs Sp %d", globaluartbaudrate);

  Wire.begin(21, 22, globali2cFrequency);
  Wire.setClock(globali2cFrequency);
  Serial1.printf("[UARTtoI2C] Khởi tạo I2C tần số %luKHz\n", globali2cFrequency);

  // initialize BH1750
  if (!bh.begin()) {
    Serial1.println("[UARTtoI2C] Không tìm thấy BH1750!");
  } else {
    Serial1.println("[UARTtoI2C] BH1750 khởi động thành công.");
  }
  lcd.printStatus("UART", "I2C", globali2cFrequency);
  delay(100);
}


void UARTtoI2C_loop() {
  static unsigned long lastReadTime = 0;
  unsigned long readInterval = 1000; 
  
    if (globali2cFrequency == 100000) {
    readInterval = 10000;
  } else if (globali2cFrequency == 400000) {
    readInterval = 4000;
  } else if (globali2cFrequency == 1000000) {
    readInterval == 1000;
  }  

  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();
    unsigned long readTimeMs = 0;
    float lux = bh.readLux(readTimeMs);

    if (lux >= 0) {
      Serial1.print("[UARTtoI2C] Freq: ");
      Serial1.print(globali2cFrequency / 1000);
      Serial1.print("KHz. ");
      Serial1.printf("Lux: %.2flx qua UART1\n", lux);
    } else {
      Serial1.println("Error:BH1750_READ_FAIL");
      Serial1.println("[UARTtoI2C] Lỗi: Không đọc được BH1750.");
    }
  }
}