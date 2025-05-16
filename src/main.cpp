#include <Arduino.h>
#include "LcdDisplay.h"
#include "USB2UART.h"
#include "USBtoI2C.h"
#include "USBto1Wire.h"

#define BUTTON_PIN 0

enum Mode {
  MODE_USB_UART,
  MODE_UART_UART,
  MODE_I2C,
  MODE_UART_I2C, /// missing
  MODE_ONEWIRE,
  MODE_UART_ONEWIRE, /// missing
  MODE_SPI,  /// missing
  MODE_UART_SPI, /// missing
  MODE_COUNT
};

/// git add -A ; git commit -m "change message! "; git push -u origin master

Mode currentMode = MODE_USB_UART;
LcdDisplay lcd;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lcd.init();
  USB2UART_setup();
  UART1_VS_UART2_setup();
}

void loop() {
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW) {
    delay(200); // debounce
    currentMode = (Mode)((currentMode + 1) % MODE_COUNT);
    Serial.print("Chuyển sang chế độ: ");
    Serial.println(currentMode);

    if (currentMode == MODE_UART_UART || currentMode == MODE_USB_UART) {
        USB2UART_setup();
        UART1_VS_UART2_setup();
    }
    else if (currentMode == MODE_I2C) USBtoI2C_setup();
    else if (currentMode == MODE_ONEWIRE) USBto1Wire_setup();
  }
  lastButtonState = buttonState;

  if (currentMode == MODE_UART_UART) UART2UART_loop();
  else if (currentMode == MODE_USB_UART) USB2UART_loop();
  else if (currentMode == MODE_I2C) USBtoI2C_loop();
  else if (currentMode == MODE_ONEWIRE) USBto1Wire_loop();

  delay(10);
}
