#include <Arduino.h>
#include "USB2UART.h"
#include "LcdDisplay.h"

extern LcdDisplay lcd;
static uint32_t uartBaud = 115200; // Tốc độ truyền UART

void USB2UART_setup() {
  Serial2.begin(uartBaud, SERIAL_8N1, 16, 17);  // RX, TX
  lcd.printStatus("USB", "UART", uartBaud);
  Serial.println("[USB2UART] Khởi động UART thành công.");
}

void USB2UART_loop() {
  if (Serial.available()) {
    Serial2.write(Serial.read());
  }
  if (Serial2.available()) {
    Serial.write(Serial2.read());
  }

  lcd.setCursor(0, 0);
  lcd.print("IN:USB  OUT:UART");
  lcd.setCursor(0, 1);
  lcd.print("SP:");
  lcd.print(uartBaud / 1000);
  lcd.print("kHz");
  delay(1000);
}
