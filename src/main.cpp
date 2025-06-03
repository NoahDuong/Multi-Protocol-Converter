#include <Arduino.h>
#include "LcdDisplay.h"
#include "USB2UART.h"
#include "USBtoI2C.h"
#include "USBto1Wire.h"
#include "USBtoSPI.h"
#include "UARTtoI2C.h"
#include "UARTto1Wire.h"
#include "UARTtoSPI.h"
#include "USB2UART.cpp"

// Định nghĩa chân GPIO cho các nút nhấn
#define BUTTON_INPUT_SOURCE      0  // Chọn ngõ vào USB/UART1
#define BUTTON_OUTPUT_PROTOCOL   4  // Chọn giao thức ngõ ra UART2/I2C/SPI/1-Wire
#define BUTTON_INPUT_SPEED       15  // Điều chỉnh tốc độ ngõ vào UART (globaluartbaudrate)
#define BUTTON_OUTPUT_SPEED      2 // Điều chỉnh tốc độ ngõ ra (I2C, SPI, 1-Wire)

// Enum cho nguồn vào
enum InputSourceMode {
  INPUT_SOURCE_USB,
  INPUT_SOURCE_UART,
  INPUT_SOURCE_COUNT
};

// Enum cho giao thức ngõ ra
enum OutputProtocolMode {
  PROTOCOL_UART,    // UART1 <-> UART2 hoặc USB <-> UART2
  PROTOCOL_I2C,
  PROTOCOL_SPI,
  PROTOCOL_ONEWIRE,
  OUTPUT_PROTOCOL_COUNT
};

// Mảng các tốc độ/tần số có thể lựa chọn
const uint32_t uartBaudRates[] = {9600, 57600, 115200, 230400};
const uint32_t i2cFrequencies[] = {100000, 400000, 1000000};
const uint32_t spiFrequencies[] = {1000000, 4000000, 8000000, 10000000};
const uint32_t oneWireSpeeds[] = {16300};

// Biến tốc độ truyền tải toàn cục - sẽ được cập nhật từ các mảng trên
uint32_t globaluartbaudrate = uartBaudRates[2]; // Mặc định 115200
uint32_t globali2cFrequency = i2cFrequencies[1]; // Mặc định 400kHz
uint32_t globalspiFrequency = spiFrequencies[0];   // Mặc định 1MHz
uint32_t onewirespeed = oneWireSpeeds[0];       // Mặc định

// Biến trạng thái hiện tại
volatile InputSourceMode currentInputSource = INPUT_SOURCE_USB;
volatile OutputProtocolMode currentOutputProtocol = PROTOCOL_UART;

// Chỉ số cho các mảng tốc độ/tần số
volatile int currentUartBaudRateIndex = 2;
volatile int currentI2cFrequencyIndex = 1;
volatile int currentSpiFrequencyIndex = 0;
volatile int currentOneWireSpeedIndex = 0; // Hiện tại chỉ có 1 lựa chọn

// Biến cờ cho các nút nhấn
volatile bool inputSourceButtonPressed = false;
volatile bool outputProtocolButtonPressed = false;
volatile bool inputSpeedButtonPressed = false;
volatile bool outputSpeedButtonPressed = false;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;

LcdDisplay lcd;

// --- ISRs cho các nút nhấn ---
void IRAM_ATTR ISR_inputSourceButton() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    inputSourceButtonPressed = true;
    lastDebounceTime = millis();
  }
}

void IRAM_ATTR ISR_outputProtocolButton() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    outputProtocolButtonPressed = true;
    lastDebounceTime = millis();
  }
}

void IRAM_ATTR ISR_inputSpeedButton() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    inputSpeedButtonPressed = true;
    lastDebounceTime = millis();
  }
}

void IRAM_ATTR ISR_outputSpeedButton() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    outputSpeedButtonPressed = true;
    lastDebounceTime = millis();
  }
}

// --- Hàm cập nhật hiển thị LCD ---
void updateDisplay() {
  lcd.clear();
  String inputStr = (currentInputSource == INPUT_SOURCE_USB) ? "USB" : "UART1";
  String outputStr = "";
  String speedStr = "";

  switch (currentOutputProtocol) {
    case PROTOCOL_UART:
      outputStr = "UART2";
      speedStr = String(globaluartbaudrate / 1000) + "k"; // Hiển thị tốc độ UART đang dùng cho output
      break;
    case PROTOCOL_I2C:
      outputStr = "I2C";
      speedStr = String(globali2cFrequency / 1000) + "k";
      break;
    case PROTOCOL_SPI:
      outputStr = "SPI";
      speedStr = String(globalspiFrequency / 1000) + "k"; // Sẽ là MHz nếu > 1000k
      if (globalspiFrequency >= 1000000) speedStr = String(globalspiFrequency / 1000000) + "M";
      break;
    case PROTOCOL_ONEWIRE:
      outputStr = "1WIRE";
      speedStr = String(onewirespeed / 1000) + "k";
      break;
  }
  // Dòng 1: IN: <source> OUT: <protocol>
  lcd.setCursor(0, 0);
  lcd.print("IN:" + inputStr + " OUT:" + outputStr);

  // Dòng 2: ISP: <input_speed> OSP: <output_speed>
  lcd.setCursor(0, 1);
  String inputSpeedDisplay = String(globaluartbaudrate / 1000) + "k";
  if (globaluartbaudrate >= 1000000) inputSpeedDisplay = String(globaluartbaudrate / 1000000) + "M";
  
  lcd.print("Is:" + inputSpeedDisplay + " Os:" + speedStr);
}


// --- Hàm áp dụng cài đặt và khởi tạo lại ngoại vi ---
void applySettingsAndInitialize() {
  Serial.println("\n[SYSTEM] Applying new settings...");
  Serial.printf("Input Source: %s\n", (currentInputSource == INPUT_SOURCE_USB) ? "USB" : "UART1");
  Serial.printf("Output Protocol: %d\n", currentOutputProtocol);
  Serial.printf("UART Baudrate: %lu\n", globaluartbaudrate);
  Serial.printf("I2C Frequency: %lu\n", globali2cFrequency);
  Serial.printf("SPI Frequency: %lu\n", globalspiFrequency);
  Serial.printf("1-Wire Speed: %lu\n", onewirespeed);
  Serial1.begin(globaluartbaudrate, SERIAL_8N1, UART1_RX, UART1_TX); // Đảm bảo UART1 input được cập nhật
  Serial.printf("[SYSTEM] UART1 (input) re-initialized to %lu bps\n", globaluartbaudrate);
  
  // Khởi tạo lại Wire với tần số mới nếu cần cho I2C (cho LCD và cảm biến)
  // TCA9548A.h sử dụng Wire, nên Wire cần được cấu hình đúng.
  // Các module I2C (Bh1750) sẽ sử dụng tần số này.
  Wire.setClock(globali2cFrequency); // Đảm bảo tần số I2C được cập nhật cho LCD và BH1750
                                    // nếu chúng được truy cập trước khi module I2C cụ thể được setup.

  // Dựa trên nguồn vào và giao thức ra để gọi hàm setup tương ứng
  if (currentInputSource == INPUT_SOURCE_USB) {
    switch (currentOutputProtocol) {
      case PROTOCOL_UART:
        USB2UART_setup(); // Sử dụng globaluartbaudrate cho Serial2
        break;
      case PROTOCOL_I2C:
        // Wire.begin đã được gọi trong setup() chính, chỉ cần setClock và gọi setup của module
        // Wire.setClock(globali2cFrequency); // Đã gọi ở trên
        USBtoI2C_setup(); // Sử dụng globali2cFrequency
        break;
      case PROTOCOL_SPI:
        USBtoSPI_setup(); // Sử dụng globalspiFrequency
        break;
      case PROTOCOL_ONEWIRE:
        USBto1Wire_setup(); // Sử dụng onewirespeed
        break;
    }
  } else {
    switch (currentOutputProtocol) {
      case PROTOCOL_UART:
        UART1_VS_UART2_setup(); // Sẽ setup Serial1 và Serial2 (cho chế độ UART-UART)
        break;
      case PROTOCOL_I2C:
        // Wire.setClock(globali2cFrequency); // Đã gọi ở trên
        UARTtoI2C_setup(); // Sử dụng globaluartbaudrate cho Serial1 (input), globali2cFrequency cho I2C
        break;
      case PROTOCOL_SPI:
        UARTtoSPI_setup(); // Sử dụng globaluartbaudrate cho Serial1 (input), globalspiFrequency cho SPI
        break;
      case PROTOCOL_ONEWIRE:
        UARTto1Wire_setup(); // Sử dụng globaluartbaudrate cho Serial1 (input), onewirespeed
        break;
    }
  }
  updateDisplay();
  Serial.println("[SYSTEM] Settings applied and peripherals initialized.");
  delay(100); // Thời gian để ổn định
}

void setup() {
  Serial.begin(globaluartbaudrate);
  while (!Serial && millis() < 3000); // Đợi Serial Monitor
  Serial.println("\n[SYSTEM] Booting up...");

  Serial1.begin(globaluartbaudrate, SERIAL_8N1, UART1_RX, UART1_TX);
  if (Serial1) {
    Serial.println("[main::setup() on COM6] Serial1 (COM8) initialized. Sending test message to COM8...");
    Serial1.println("[main::setup() on COM8] Serial1 (COM8) TEST PRINT - If you see this, Serial1 in setup() is OK!");
  } else {
    Serial.println("[main::setup() on COM6] Serial1 (COM8) FAILED to initialize!");
  }
  // Khởi tạo I2C bus ban đầu cho LCD và các thiết bị I2C khác
  // Chân SDA, SCL mặc định của ESP32 thường là 21, 22
  Wire.begin(21, 22, globali2cFrequency); // Khởi tạo Wire với tần số mặc định
  Serial.println("[SYSTEM] I2C bus initialized.");

  // Cấu hình các nút nhấn
  pinMode(BUTTON_INPUT_SOURCE, INPUT_PULLUP);
  pinMode(BUTTON_OUTPUT_PROTOCOL, INPUT_PULLUP);
  pinMode(BUTTON_INPUT_SPEED, INPUT_PULLUP);
  pinMode(BUTTON_OUTPUT_SPEED, INPUT_PULLUP);

  // Đính kèm ngắt cho các nút nhấn
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_SOURCE), ISR_inputSourceButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_OUTPUT_PROTOCOL), ISR_outputProtocolButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_SPEED), ISR_inputSpeedButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_OUTPUT_SPEED), ISR_outputSpeedButton, FALLING);
  Serial.println("[SYSTEM] Buttons initialized with interrupts.");

  // Khởi tạo LCD
  lcd.init(); // Hàm này sẽ tự động chọn kênh LCD qua tcaselect
  Serial.println("[SYSTEM] LCD initialized.");

  // Khởi tạo chế độ mặc định
  applySettingsAndInitialize();

  Serial.println("[SYSTEM] Setup complete. Default mode active.");
}

void loop() {
  bool settingsChanged = false;

  // Xử lý nút chọn nguồn vào
  if (inputSourceButtonPressed) {
    inputSourceButtonPressed = false;
    currentInputSource = (InputSourceMode)((currentInputSource + 1) % INPUT_SOURCE_COUNT);
    Serial.printf("[BTN] Input Source changed to: %d\n", currentInputSource);
    settingsChanged = true;
  }

  // Xử lý nút chọn giao thức ngõ ra
  if (outputProtocolButtonPressed) {
    outputProtocolButtonPressed = false;
    currentOutputProtocol = (OutputProtocolMode)((currentOutputProtocol + 1) % OUTPUT_PROTOCOL_COUNT);
    Serial.printf("[BTN] Output Protocol changed to: %d\n", currentOutputProtocol);
    settingsChanged = true;
  }

  // Xử lý nút thay đổi tốc độ ngõ vào (UART)
  if (inputSpeedButtonPressed) {
    inputSpeedButtonPressed = false;
    currentUartBaudRateIndex = (currentUartBaudRateIndex + 1) % (sizeof(uartBaudRates) / sizeof(uartBaudRates[0]));
    globaluartbaudrate = uartBaudRates[currentUartBaudRateIndex];
    Serial.printf("[BTN] Input UART Baudrate changed to: %lu\n", globaluartbaudrate);
    settingsChanged = true;
  }

  // Xử lý nút thay đổi tốc độ ngõ ra
  if (outputSpeedButtonPressed) {
    outputSpeedButtonPressed = false;
    switch (currentOutputProtocol) {
      case PROTOCOL_UART: // Nếu ngõ ra là UART (UART-UART mode)
        // Trong trường hợp này, nút này cũng có thể điều chỉnh globaluartbaudrate
        // hoặc một biến riêng cho tốc độ UART2 nếu muốn phân biệt.
        // Để đơn giản, ở đây cũng thay đổi globaluartbaudrate, ảnh hưởng đến cả Serial1 và Serial2 trong chế độ UART-UART.
        currentUartBaudRateIndex = (currentUartBaudRateIndex + 1) % (sizeof(uartBaudRates) / sizeof(uartBaudRates[0]));
        globaluartbaudrate = uartBaudRates[currentUartBaudRateIndex];
        Serial.printf("[BTN] Output UART (UART2) Baudrate (global) changed to: %lu\n", globaluartbaudrate);
        break;
      case PROTOCOL_I2C:
        currentI2cFrequencyIndex = (currentI2cFrequencyIndex + 1) % (sizeof(i2cFrequencies) / sizeof(i2cFrequencies[0]));
        globali2cFrequency = i2cFrequencies[currentI2cFrequencyIndex];
        Serial.printf("[BTN] I2C Frequency changed to: %lu\n", globali2cFrequency);
        break;
      case PROTOCOL_SPI:
        currentSpiFrequencyIndex = (currentSpiFrequencyIndex + 1) % (sizeof(spiFrequencies) / sizeof(spiFrequencies[0]));
        globalspiFrequency = spiFrequencies[currentSpiFrequencyIndex];
        Serial.printf("[BTN] SPI Frequency changed to: %lu\n", globalspiFrequency);
        break;
      case PROTOCOL_ONEWIRE:
        currentOneWireSpeedIndex = (currentOneWireSpeedIndex + 1) % (sizeof(oneWireSpeeds) / sizeof(oneWireSpeeds[0]));
        onewirespeed = oneWireSpeeds[currentOneWireSpeedIndex];
        Serial.printf("[BTN] 1-Wire Speed changed to: %lu (Note: may be fixed by library)\n", onewirespeed);
        break;
    }
    settingsChanged = true;
  }

  // Nếu có thay đổi cài đặt, áp dụng và khởi tạo lại
  if (settingsChanged) {
    applySettingsAndInitialize();
  }

  // Thực thi loop của chế độ hiện tại
  if (currentInputSource == INPUT_SOURCE_USB) {
    switch (currentOutputProtocol) {
      case PROTOCOL_UART:    USB2UART_loop(); break;
      case PROTOCOL_I2C:     USBtoI2C_loop(); break;
      case PROTOCOL_SPI:     USBtoSPI_loop(); break;
      case PROTOCOL_ONEWIRE: USBto1Wire_loop(); break;
    }
  } else { // INPUT_SOURCE_UART
    switch (currentOutputProtocol) {
      case PROTOCOL_UART:    UART2UART_loop(); break;
      case PROTOCOL_I2C:     UARTtoI2C_loop(); break;
      case PROTOCOL_SPI:     UARTtoSPI_loop(); break;
      case PROTOCOL_ONEWIRE: UARTto1Wire_loop(); break;
    }
  }

  delay(10); // Giảm tải CPU
}