#include <Arduino.h>
#include "USB2UART.h"
#include "LcdDisplay.h" // Mặc dù LCD đã bị comment, include vẫn có thể cần thiết cho khai báo extern LcdDisplay

extern LcdDisplay lcd; // Vẫn khai báo, dù các lệnh lcd.print đã bị comment trong file này
#define UART1_TX 26
#define UART1_RX 27

void USB2UART_setup() {
  // Serial2 sử dụng chân mặc định của ESP32 là GPIO16 (RX2) và GPIO17 (TX2)
  Serial2.begin(globaluartbaudrate, SERIAL_8N1, 16, 17);
  Serial.printf("[USB2UART] UART2 (output for USB mode) online with Speed %lu.\n", globaluartbaudrate);
  // Phần LCD đã được comment theo "Cách 2"
  // lcd.printStatus("USB", "UART", globaluartbaudrate);
}

void UART1_VS_UART2_setup() {
  // Serial1 đã được khởi tạo bởi main.cpp trong hàm applySettingsAndInitialize()
  // trước khi hàm này được gọi, với globaluartbaudrate và các chân UART1_RX, UART1_TX.
  // Vì vậy, không cần gọi lại Serial1.begin() ở đây.
  Serial.printf("[USB2UART] UART1 (for UART-UART mode) should be already initialized by main.cpp to %lu bps.\n", globaluartbaudrate);

  // Cần đảm bảo Serial2 cũng được cấu hình với globaluartbaudrate
  // trong chế độ UART1 <-> UART2.
  Serial2.begin(globaluartbaudrate, SERIAL_8N1, 16, 17); // RX2, TX2
  Serial.printf("[USB2UART] UART2 (for UART-UART mode) re-initialized to %lu bps.\n", globaluartbaudrate);

  // Phần LCD đã được comment theo "Cách 2"
  // lcd.printStatus("UART1","UART2",globaluartbaudrate); // Ví dụ nếu có hàm printStatus mới
}

void UART2UART_loop() { // Chế độ UART1 <-> UART2
  if (Serial1.available()) {
    char rcv = Serial1.read();
    Serial2.write(rcv);
    Serial.write(rcv); // Gửi lên Serial Monitor debug
  }
  if (Serial2.available()) {
    char rcv = Serial2.read();
    Serial1.write(rcv);
    Serial.write(rcv); // Gửi lên Serial Monitor debug
  }

  // Phần LCD đã được comment theo "Cách 2"
  // lcd.setCursor(0, 0);
  // lcd.print("IN:UART OUT:UART");
  // lcd.setCursor(0, 1);
  // lcd.print("SP:");
  // lcd.print(globaluartbaudrate/ 1000);
  // lcd.print("kHz");
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

  // Phần LCD đã được comment theo "Cách 2"
  // lcd.setCursor(0, 0);
  // lcd.print("IN:USB  OUT:UART");
  // lcd.setCursor(0, 1);
  // lcd.print("SP:");
  // lcd.print(globaluartbaudrate/ 1000);
  // lcd.print("kHz");
}

// Hàm UART1_TO_UART2() có thể không cần thiết nếu UART2UART_loop() đã xử lý
// Tuy nhiên, nếu bạn có mục đích sử dụng riêng, bạn có thể giữ lại hoặc xóa đi.
// Nếu UART1_VS_UART2_setup và UART2UART_loop là đủ cho chế độ UART1<->UART2 thì hàm này có thể không dùng đến.
void UART1_TO_UART2() {
    // Logic này tương tự như trong UART2UART_loop, có thể đã thừa.
    if (Serial1.available()) {
        Serial2.write(Serial1.read());
    }
    if (Serial2.available()) {
        Serial1.write(Serial2.read());
    }
}