#include <Arduino.h>
#include <SPI.h>
#include "USBtoSPI.h"
#include "LcdDisplay.h"

extern LcdDisplay lcd;
static uint32_t uartBaud = 115200; // Tốc độ truyền UART

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
float x_offset= 0, y_offset= 0, z_offset= 0;
const int CALIB_SAMPLES=100;

// Khởi tạo đối tượng SPI
SPIClass *spi_adxl345 = NULL; // Con trỏ đến đối tượng SPIClass

// Tốc độ SPI (ADXL345 hỗ trợ tới 5 MHz, dùng 4 MHz an toàn)
static uint32_t spiSpeed = 4000000; // 4 MHz

// Hàm ghi một byte vào thanh ghi của ADXL345
void writeRegister(byte regAddress, byte value) {
  spi_adxl345->beginTransaction(SPISettings(spiSpeed, MSBFIRST, SPI_MODE3)); // Tốc độ SPI, thứ tự bit, chế độ SPI (ADXL345 dùng Mode 0 hoặc 3)
  digitalWrite(ADXL345_SS_PIN, LOW);   // Kích hoạt Chip Select
  spi_adxl345->transfer(regAddress);    // Gửi địa chỉ thanh ghi
  spi_adxl345->transfer(value);         // Gửi giá trị
  digitalWrite(ADXL345_SS_PIN, HIGH);  // Ngắt kích hoạt Chip Select
  spi_adxl345->endTransaction();
}

// Hàm đọc một byte từ thanh ghi của ADXL345
byte readRegister(byte regAddress) {
  byte value = 0;
  spi_adxl345->beginTransaction(SPISettings(spiSpeed, MSBFIRST, SPI_MODE3));
  digitalWrite(ADXL345_SS_PIN, LOW);
  spi_adxl345->transfer(regAddress | 0x80); // Đặt bit đọc (MSB = 1)
  value = spi_adxl345->transfer(0x00);      // Gửi byte dummy để nhận dữ liệu
  digitalWrite(ADXL345_SS_PIN, HIGH);
  spi_adxl345->endTransaction();
  return value;
}

// Hàm đọc nhiều byte từ các thanh ghi liên tiếp của ADXL345
void readRegisters(byte regAddress, byte numBytes, byte *buffer) {
  spi_adxl345->beginTransaction(SPISettings(spiSpeed, MSBFIRST, SPI_MODE3));
  digitalWrite(ADXL345_SS_PIN, LOW);
  spi_adxl345->transfer(regAddress | 0xC0); // Đặt bit đọc (MSB = 1) và bit đa byte (bit 6 = 1)
  for (int i = 0; i < numBytes; i++) {
    buffer[i] = spi_adxl345->transfer(0x00); // Gửi byte dummy để nhận dữ liệu
  }
  digitalWrite(ADXL345_SS_PIN, HIGH);
  spi_adxl345->endTransaction();
}

void calibrateStatic(){
  long sumX=0, sumY=0, sumZ=0;
  byte raw[6];
  for (int i=0; i< CALIB_SAMPLES; i++){
    readRegisters(ADXL345_DATAX0, 6, raw);
    int16_t xr = (raw[1] << 8) | raw[0];
    int16_t yr = (raw[3] << 8) | raw[2];
    int16_t zr = (raw[5] << 8) | raw[4];
    sumX += xr; sumY+= yr; sumZ += zr;
    delay(10);
  }
  x_offset = (sumX/(float)CALIB_SAMPLES)/ADXL345_LSB_PER_G;
  y_offset = (sumY/(float)CALIB_SAMPLES)/ADXL345_LSB_PER_G;
  z_offset = (sumZ/(float)CALIB_SAMPLES)/ADXL345_LSB_PER_G;
}

void USBtoSPI_setup() {
  Serial.println("[USBtoSPI] Khởi tạo ADXL345 qua SPI...");
  // Cấu hình chân CS
  pinMode(ADXL345_SS_PIN, OUTPUT);
  digitalWrite(ADXL345_SS_PIN, HIGH); // CS ban đầu ở mức cao

  // Khởi tạo SPI bus (VSPI)
  if (spi_adxl345 == NULL) { // Chỉ khởi tạo một lần
    spi_adxl345 = new SPIClass(VSPI);
  }
  spi_adxl345->begin(ADXL345_SCK_PIN, ADXL345_MISO_PIN, ADXL345_MOSI_PIN, ADXL345_SS_PIN); // SCK, MISO, MOSI, SS

  // Kiểm tra kết nối ADXL345 bằng cách đọc DEVID
  byte deviceID = readRegister(ADXL345_DEVID);
  Serial.print("[USBtoSPI] Device ID: 0x");
  Serial.println(deviceID, HEX);

  if (deviceID != 0xE5) {
    Serial.println("[USBtoSPI] Khong tim thay ADXL345! Vui long kiem tra ket noi.");
    // Có thể thêm vòng lặp while(true) hoặc cờ lỗi để dừng nếu không tìm thấy
  } else {
    Serial.println("[USBtoSPI] ADXL345 da duoc tim thay.");
    // Cấu hình ADXL345
    writeRegister(ADXL345_DATA_FORMAT, ADXL345_FULL_RES | ADXL345_RANGE_2G);
    writeRegister(ADXL345_POWER_CTL, ADXL345_MEASURE);
    Serial.println("[USBtoSPI] ADXL345 da duoc cau hinh.");
  }
  Serial.println("Gia tri tinh...");
  calibrateStatic();
  Serial.printf("Offsets: X=%.3f Y=%3f Z=%3f\n", x_offset,  y_offset, z_offset);
  lcd.printStatus("USB", "SPI", spiSpeed);
  delay(100); // Đợi một chút để cảm biến khởi động
}

void USBtoSPI_loop() {
  byte rawData[6];
  readRegisters(ADXL345_DATAX0, 6, rawData);

  // Kết hợp các byte thấp và cao thành giá trị 16-bit
  int16_t x_raw = ((int16_t)rawData[1] << 8) | rawData[0];
  int16_t y_raw = ((int16_t)rawData[3] << 8) | rawData[2];
  int16_t z_raw = ((int16_t)rawData[5] << 8) | rawData[4];

  float x_g = (float)x_raw / ADXL345_LSB_PER_G;
  float y_g = (float)y_raw / ADXL345_LSB_PER_G;
  float z_g = (float)z_raw / ADXL345_LSB_PER_G;

  float x_dyn = x_g - x_offset;
  float y_dyn = y_g - y_offset;
  float z_dyn = z_g - z_offset;

  float a_dyn = sqrt(x_dyn*x_dyn + y_dyn*y_dyn + z_dyn*z_dyn);
  // Hiển thị lên Serial Monitor
  Serial.print("[USBtoSPI] Gia tri thap (X, Y, Z): ");
  Serial.print(x_raw);
  Serial.print("\t");
  Serial.print(y_raw);
  Serial.print("\t");
  Serial.println(z_raw);

  Serial.print("[USBtoSPI] Gia toc (X, Y, Z): ");
  Serial.print(x_g, 2);
  Serial.print("g\t");
  Serial.print(y_g, 2);
  Serial.print("g\t");
  Serial.print(z_g, 2);
  Serial.println("g");

  Serial.printf("Gia toc dong: %.3f g\n", a_dyn);
  // Hiển thị lên LCD
  lcd.setCursor(0, 0);
  lcd.print("IN:USB   OUT:SPI");
  
  lcd.setCursor(0, 1);
  lcd.print("SP:");
  lcd.print(uartBaud/1000);
  lcd.print("KHz ");
  lcd.print("A=");
  lcd.print(a_dyn, 1);
  lcd.print("g");

  delay(300);
}