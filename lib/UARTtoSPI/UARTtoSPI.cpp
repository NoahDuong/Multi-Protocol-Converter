#include <Arduino.h>
#include <SPI.h>
#include "UARTtoSPI.h"
#include "LcdDisplay.h"

extern LcdDisplay lcd;
#define ADXL345_SS_PIN      5   // Chip Select (CS) pin cho ADXL345
#define ADXL345_SCK_PIN     18  // SPI Clock (SCK)
#define ADXL345_MISO_PIN    19  // Master In Slave Out (MISO)
#define ADXL345_MOSI_PIN    23  // Master Out Slave In (MOSI)

#define ADXL345_DEVID         0x00 // Device ID
#define ADXL345_POWER_CTL     0x2D // Power-saving features control
#define ADXL345_DATA_FORMAT   0x31 // Data format control
#define ADXL345_DATAX0        0x32 // X-axis data 0
#define ADXL345_MEASURE       0x08 // Chế độ đo lường (Measurement Mode)
#define ADXL345_FULL_RES      0x08 // Full resolution (10-bit to 13-bit for 2g to 16g ranges)
#define ADXL345_RANGE_2G      0x00 // +/- 2g

// Hệ số chuyển đổi từ giá trị thô sang gia tốc (g).
const float ADXL345_LSB_PER_G = 256.0; // Đối với dải +/-2g, full resolution
float X_uart_offset = 0, y_uart_offset = 0, z_uart_offset = 0;
const int CALIB_SAMPLES = 100;

// Khởi tạo đối tượng SPI
SPIClass *spi_adxl345_uart = NULL; // Dùng tên khác để tránh trùng với USBtoSPI

// Cấu hình UART
#define UART1_TX_PIN 26 // Chân TX của UART1
#define UART1_RX_PIN 27 // Chân RX của UART1

// Hàm ghi một byte vào thanh ghi của ADXL345
void writeRegister_UART(byte regAddress, byte value) {
  spi_adxl345_uart->beginTransaction(SPISettings(globalspiFrequency, MSBFIRST, SPI_MODE3));
  digitalWrite(ADXL345_SS_PIN, LOW);
  spi_adxl345_uart->transfer(regAddress);
  spi_adxl345_uart->transfer(value);
  digitalWrite(ADXL345_SS_PIN, HIGH);
  spi_adxl345_uart->endTransaction();
}

// Hàm đọc một byte từ thanh ghi của ADXL345 (phiên bản cho UARTtoSPI)
byte readRegister_UART(byte regAddress) {
  byte value = 0;
  spi_adxl345_uart->beginTransaction(SPISettings(globalspiFrequency, MSBFIRST, SPI_MODE3));
  digitalWrite(ADXL345_SS_PIN, LOW);
  spi_adxl345_uart->transfer(regAddress | 0x80);
  value = spi_adxl345_uart->transfer(0x00);
  digitalWrite(ADXL345_SS_PIN, HIGH);
  spi_adxl345_uart->endTransaction();
  return value;
}

// Hàm đọc nhiều byte từ các thanh ghi liên tiếp của ADXL345 (phiên bản cho UARTtoSPI)
void readRegisters_UART(byte regAddress, byte numBytes, byte *buffer) {
  spi_adxl345_uart->beginTransaction(SPISettings(globalspiFrequency, MSBFIRST, SPI_MODE3));
  digitalWrite(ADXL345_SS_PIN, LOW);
  spi_adxl345_uart->transfer(regAddress | 0xC0);
  for (int i = 0; i < numBytes; i++) {
    buffer[i] = spi_adxl345_uart->transfer(0x00);
  }
  digitalWrite(ADXL345_SS_PIN, HIGH);
  spi_adxl345_uart->endTransaction();
}

void uart_calibrateStatic(){
  long sumX=0, sumY=0, sumZ=0;
  byte raw[6];
  for (int i=0; i< CALIB_SAMPLES; i++){
    readRegisters_UART(ADXL345_DATAX0, 6, raw);
    int16_t xr = (raw[1] << 8) | raw[0];
    int16_t yr = (raw[3] << 8) | raw[2];
    int16_t zr = (raw[5] << 8) | raw[4];
    sumX += xr; sumY+= yr; sumZ += zr;
    delay(10);
  }
  X_uart_offset = (sumX/(float)CALIB_SAMPLES)/ADXL345_LSB_PER_G;
  y_uart_offset = (sumY/(float)CALIB_SAMPLES)/ADXL345_LSB_PER_G;
  z_uart_offset = (sumZ/(float)CALIB_SAMPLES)/ADXL345_LSB_PER_G;
}

void UARTtoSPI_setup() {
  Serial.println("[UARTtoSPI] Khởi tạo ADXL345 qua SPI...");

  // Khởi tạo UART1 nếu chưa được khởi tạo
  Serial1.begin(globaluartbaudrate, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);
  Serial.println("[UARTtoSPI] Khởi động UART1 thành công.");

  // Cấu hình chân CS
  pinMode(ADXL345_SS_PIN, OUTPUT);
  digitalWrite(ADXL345_SS_PIN, HIGH); // CS ban đầu ở mức cao

  // Khởi tạo SPI bus (VSPI)
  if (spi_adxl345_uart == NULL) { // Chỉ khởi tạo một lần
    spi_adxl345_uart = new SPIClass(VSPI);
  }
  spi_adxl345_uart->begin(ADXL345_SCK_PIN, ADXL345_MISO_PIN, ADXL345_MOSI_PIN, ADXL345_SS_PIN); // SCK, MISO, MOSI, SS

  // Kiểm tra kết nối ADXL345 bằng cách đọc DEVID
  byte deviceID = readRegister_UART(ADXL345_DEVID);
  Serial.print("[UARTtoSPI] Device ID: 0x");
  Serial.println(deviceID, HEX);

  if (deviceID != 0xE5) {
    Serial.println("[UARTtoSPI] Khong tim thay ADXL345! Vui long kiem tra ket noi.");
  } else {
    Serial.println("[UARTtoSPI] ADXL345 da duoc tim thay.");
    // Cấu hình ADXL345
    writeRegister_UART(ADXL345_DATA_FORMAT, ADXL345_FULL_RES | ADXL345_RANGE_2G);
    writeRegister_UART(ADXL345_POWER_CTL, ADXL345_MEASURE);
    Serial.println("[UARTtoSPI] ADXL345 da duoc cau hinh.");
  }
  Serial.println("Gia tri tinh...");
  uart_calibrateStatic();
  Serial.printf("Offsets: X=%.3f Y=%3f Z=%3f\n", X_uart_offset,  y_uart_offset, z_uart_offset);
  lcd.printStatus("UART", "SPI", globalspiFrequency);
  delay(100);
}

void UARTtoSPI_loop() {
  byte rawData[6];
  readRegisters_UART(ADXL345_DATAX0, 6, rawData);

  // Kết hợp các byte thấp và cao thành giá trị 16-bit
  int16_t x_raw = ((int16_t)rawData[1] << 8) | rawData[0];
  int16_t y_raw = ((int16_t)rawData[3] << 8) | rawData[2];
  int16_t z_raw = ((int16_t)rawData[5] << 8) | rawData[4];

  float x_g = (float)x_raw / ADXL345_LSB_PER_G;
  float y_g = (float)y_raw / ADXL345_LSB_PER_G;
  float z_g = (float)z_raw / ADXL345_LSB_PER_G;

  float x_dyn = x_g - X_uart_offset;
  float y_dyn = y_g - y_uart_offset;
  float z_dyn = z_g - z_uart_offset;

  float a_dyn = sqrt(x_dyn*x_dyn + y_dyn*y_dyn + z_dyn*z_dyn);
  // Hiển thị lên Serial Monitor
  Serial.printf("Gia toc dong: %.3f g\n", a_dyn);
    // lcd.setCursor(0, 0);
    // lcd.print("IN:UART  OUT:SPI");
    // lcd.setCursor(0, 1);
    // lcd.print("SP:");
    // lcd.print(globalspiFrequency/1000);
    // lcd.print("KHz ");
    // lcd.print("A=");
    // lcd.print(a_dyn, 1);
    // lcd.print("g");

    delay(300);
}