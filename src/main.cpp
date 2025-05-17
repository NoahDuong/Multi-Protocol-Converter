#include <Arduino.h>
#include "LcdDisplay.h"
#include "USB2UART.h"
#include "USBtoI2C.h"
#include "USBto1Wire.h"
#include "USBtoSPI.h"
#include "UARTtoI2C.h"
#include "UARTto1Wire.h"
#include "UARTtoSPI.h"

#define BUTTON_USB 0     // Nút chuyển các chế độ USB → ...
#define BUTTON_UART 4    // Nút chuyển các chế độ UART → ...

// Chỉ định chế độ hoạt động
enum ModeType {
  MODE_USB,
  MODE_UART
};

enum USBMode {
  USB_UART,
  USB_I2C,
  USB_SPI,
  USB_ONEWIRE,
  USB_MODE_COUNT
};

enum UARTMode {
  UART_UART,
  UART_I2C,
  UART_SPI,
  UART_ONEWIRE,
  UART_MODE_COUNT
};

// Biến toàn cục
volatile ModeType currentModeType = MODE_USB; // Mode mặc định là USB
volatile USBMode currentUSBMode = USB_UART;
volatile UARTMode currentUARTMode = UART_UART;
volatile bool usbButtonPressed = false;
volatile bool uartButtonPressed = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;

LcdDisplay lcd;

// ISR handler cho nút USB
void IRAM_ATTR ISR_usbButton() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    usbButtonPressed = true;
    lastDebounceTime = millis();
  }
}

// ISR handler cho nút UART
void IRAM_ATTR ISR_uartButton() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    uartButtonPressed = true;
    lastDebounceTime = millis();
  }
}

// Xử lý chuyển chế độ USB
void processUSBMode() {
  lcd.clear();
  switch (currentUSBMode) {
    case USB_UART:
      Serial.println("Chế độ: USB → UART");
      // lcd.print("USB -> UART");
      USB2UART_setup();
      break;
    case USB_I2C:
      Serial.println("Chế độ: USB → I2C");
      // lcd.print("USB -> I2C");
      USBtoI2C_setup();
      break;
    case USB_SPI:
      Serial.println("Chế độ: USB → SPI");
      // lcd.print("USB -> SPI");
      USBtoSPI_setup();
      break;
    case USB_ONEWIRE:
      Serial.println("Chế độ: USB → 1-Wire");
      // lcd.print("USB -> 1-Wire");
      USBto1Wire_setup();
      break;
    default:
      currentUSBMode = USB_UART;
      processUSBMode();
      break;
  }
}

// Xử lý chuyển chế độ UART
void processUARTMode() {
  lcd.clear();
  switch (currentUARTMode) {
    case UART_UART:
      Serial.println("Chế độ: UART ↔ UART");
      // lcd.print("UART <-> UART");
      UART1_VS_UART2_setup();
      break;
    case UART_I2C:
      Serial.println("Chế độ: UART → I2C");
      // lcd.print("UART -> I2C");
      UARTtoI2C_setup();
      break;
    case UART_SPI:
      Serial.println("Chế độ: UART → SPI");
      // lcd.print("UART -> SPI");
      UARTtoSPI_setup();
      break;
    case UART_ONEWIRE:
      Serial.println("Chế độ: UART → 1-Wire");
      // lcd.print("UART -> 1-Wire");
      UARTto1Wire_setup();
      break;
    default:
      currentUARTMode = UART_UART;
      processUARTMode();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  // Cấu hình các nút nhấn
  pinMode(BUTTON_USB, INPUT_PULLUP);
  pinMode(BUTTON_UART, INPUT_PULLUP);

  // Đính kèm ngắt cho các nút nhấn
  attachInterrupt(digitalPinToInterrupt(BUTTON_USB), ISR_usbButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_UART), ISR_uartButton, FALLING);

  // Khởi tạo LCD
  lcd.init();

  // Khởi tạo chế độ mặc định
  currentModeType = MODE_USB;
  processUSBMode();

  Serial.println("Khởi động xong. Chế độ mặc định: USB → UART");
}

void loop() {
  // Xử lý nút USB
  if (usbButtonPressed) {
    usbButtonPressed = false;
    currentModeType = MODE_USB;
    currentUSBMode = (USBMode)((currentUSBMode + 1) % USB_MODE_COUNT);
    processUSBMode();
  }

  // Xử lý nút UART
  if (uartButtonPressed) {
    uartButtonPressed = false;
    currentModeType = MODE_UART;
    currentUARTMode = (UARTMode)((currentUARTMode + 1) % UART_MODE_COUNT);
    processUARTMode();
  }

  // Chỉ chạy loop của mode hiện tại
  if (currentModeType == MODE_USB) {
    switch (currentUSBMode) {
      case USB_UART: USB2UART_loop(); break;
      case USB_I2C: USBtoI2C_loop(); break;
      case USB_SPI: USBtoSPI_loop(); break;
      case USB_ONEWIRE: USBto1Wire_loop(); break;
    }
  } 
  else { // MODE_UART
    switch (currentUARTMode) {
      case UART_UART: UART2UART_loop(); break;
      case UART_I2C: UARTtoI2C_loop(); break;
      case UART_SPI: UARTtoSPI_loop(); break;
      case UART_ONEWIRE: UARTto1Wire_loop(); break;
    }
  }

  delay(10);
}