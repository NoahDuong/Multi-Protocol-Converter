#include <Arduino.h>
#include "USB2UART.h"
#include "LcdDisplay.h"

extern LcdDisplay lcd;

#define UART1_TX 26 // <---> CP2102.RX
#define UART1_RX 27 // <---> CP2102.TX

void USB2UART_setup() {
  Serial2.begin(globaluartbaudrate, SERIAL_8N1, 16, 17);  // RX, TX
  lcd.printStatus("USB", "UART", globaluartbaudrate);
  Serial.printf("UART2 online vs Sp %d.\n", globaluartbaudrate);
}

void UART1_VS_UART2_setup(){
  Serial1.begin(globaluartbaudrate, SERIAL_8N1, UART1_RX, UART1_TX);  // RX, TX
  Serial.printf("UART1 online vs Sp %d.\n", globaluartbaudrate);
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
  lcd.print(globaluartbaudrate/ 1000);
  lcd.print("kHz");
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
  lcd.print(globaluartbaudrate/ 1000);
  lcd.print("kHz");
}