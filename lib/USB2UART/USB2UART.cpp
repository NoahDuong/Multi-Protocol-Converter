#include <Arduino.h>
#include "USB2UART.h"
#include "LcdDisplay.h"

extern LcdDisplay lcd;
#define UART1_TX 26
#define UART1_RX 27

void USB2UART_setup() {
  Serial2.begin(globaluartbaudrate, SERIAL_8N1, 16, 17);
  Serial.printf("[USB2UART] UART2 (output for USB mode) online with Speed %lu.\n", globaluartbaudrate);
}

void UART1_VS_UART2_setup() {
  Serial1.printf("[USB2UART] UART1 online w/ %lu bps.\n", globaluartbaudrate);
  Serial2.begin(globaluartbaudrate, SERIAL_8N1, 16, 17);
  Serial1.printf("[USB2UART] UART2 online w/ %lu bps.\n", globaluartbaudrate);
}

void USB2UART_loop() { // Chế độ USB (Serial) <-> UART2 (Serial1 có thể là stalker)
  if (Serial.available()) { // Đọc từ Serial Monitor debug (ví dụ: từ máy tính)
    char rcv = Serial.read();
    Serial2.write(rcv);   // Gửi ra Serial2
    // Serial1.write(rcv); // Nếu Serial1 làm "stalker"
  }
  if (Serial2.available()) { // Đọc từ thiết bị kết nối với Serial2
    char rcv = Serial2.read();
    Serial.write(rcv);    // Gửi lên Serial Monitor debug
    // Serial1.write(rcv); // Nếu Serial1 làm "stalker"
  }
}

void UART2UART_loop() { // Chế độ UART1 <-> UART2
  if (Serial1.available()) {
    char rcv = Serial1.read();
    Serial2.write(rcv);
    Serial.write(rcv);
  }
  if (Serial2.available()) {
    char rcv = Serial2.read();
    Serial1.write(rcv);
    Serial.write(rcv);
  }
}

void UART1_TO_UART2() {
    if (Serial1.available()) {
        Serial2.write(Serial1.read());
    }
    if (Serial2.available()) {
        Serial1.write(Serial2.read());
    }
}