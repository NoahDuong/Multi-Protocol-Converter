#include <Arduino.h>
#include "USB2UART.h"
#include "LcdDisplay.h"

extern LcdDisplay lcd;
static uint32_t uartBaud = 115200; // Tốc độ truyền UART

#define UART1_TX 26 // <---> CP2102.RX
#define UART1_RX 27 // <---> CP2102.TX

void USB2UART_setup() {
  Serial2.begin(uartBaud, SERIAL_8N1, 16, 17);  // RX, TX
  lcd.printStatus("USB", "UART", uartBaud);
  Serial.println("UART2 online");
}

void UART1_VS_UART2_setup(){
  Serial1.begin(uartBaud, SERIAL_8N1, UART1_RX, UART1_TX);  // RX, TX
  Serial.println("UART1 online");
}

void UART2UART_loop() { /// uart0: stalker, uart1 <-> uart2
  if (Serial1.available()) {
    auto rcv = Serial1.read();
    Serial2.write(rcv);
    Serial.write(rcv);
  }
  if (Serial2.available()) {
    auto rcv = Serial2.read();
    Serial1.write(rcv);
    Serial.write(rcv);
  }
  lcd.setCursor(0, 0);
  lcd.print("IN:UART OUT:UART");
  lcd.setCursor(0, 1);
  lcd.print("SP:");
  lcd.print(uartBaud / 1000);
  lcd.print("kHz");
  delay(400);
}

void USB2UART_loop() { /// uart1: stalker, uart0 (WHITE) <-> uart2
  if (Serial.available()) {
    auto rcv = Serial.read();
    Serial2.write(rcv);
    Serial1.write(rcv);
  }
  if (Serial2.available()) {
    auto rcv = Serial2.read();
    Serial.write(rcv);
    Serial1.write(rcv);
  }
  lcd.setCursor(0, 0);
  lcd.print("IN:USB  OUT:UART");
  lcd.setCursor(0, 1);
  lcd.print("SP:");
  lcd.print(uartBaud / 1000);
  lcd.print("kHz");
  delay(400);
}

// void UART1_TO_UART2_loop(){
//   if (Serial.available()) {
//     Serial2.write(Serial.read());
//   }
//   if (Serial2.available()) {
//     Serial.write(Serial2.read());
//   }
// }