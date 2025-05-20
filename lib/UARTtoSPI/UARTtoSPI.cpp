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

// Khởi tạo đối tượng SPI
SPIClass *spi_adxl345_uart = NULL; // Dùng tên khác để tránh trùng với USBtoSPI

// Cấu hình UART
#define UART1_TX_PIN 26 // Chân TX của UART1
#define UART1_RX_PIN 27 // Chân RX của UART1
static uint32_t uartBaudRate = 115200; // Tốc độ baud cho UART1

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

/**
 * @brief Hàm setup cho chế độ UART to SPI (ADXL345).
 * Khởi tạo UART1, SPI bus và cấu hình cảm biến ADXL345.
 */
void UARTtoSPI_setup() {
  Serial.println("[UARTtoSPI] Khởi tạo ADXL345 qua SPI...");

  // Khởi tạo UART1 nếu chưa được khởi tạo
  Serial1.begin(uartBaudRate, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);
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
    // Có thể thêm vòng lặp while(true) hoặc cờ lỗi để dừng nếu không tìm thấy
  } else {
    Serial.println("[UARTtoSPI] ADXL345 da duoc tim thay.");
    // Cấu hình ADXL345
    writeRegister_UART(ADXL345_DATA_FORMAT, ADXL345_FULL_RES | ADXL345_RANGE_2G);
    writeRegister_UART(ADXL345_POWER_CTL, ADXL345_MEASURE);
    Serial.println("[UARTtoSPI] ADXL345 da duoc cau hinh.");
  }

  // Cập nhật trạng thái trên LCD
  lcd.printStatus("UART", "SPI", globalspiFrequency);
  delay(100);
}

/**
 * @brief Hàm loop cho chế độ UART to SPI (ADXL345).
 * Lắng nghe lệnh qua UART1 để đọc gia tốc và gửi kết quả.
 */
void UARTtoSPI_loop() {
  static unsigned long lastReadTime = 0;
  const long readInterval = 500; // Đọc mỗi 500ms để không quá tải cảm biến

  if (Serial1.available()) {
    char command = Serial1.read();
    if (command == 'A') { // Yêu cầu đọc gia tốc
      byte rawData[6];
      readRegisters_UART(ADXL345_DATAX0, 6, rawData);

      int16_t x_raw = ((int16_t)rawData[1] << 8) | rawData[0];
      int16_t y_raw = ((int16_t)rawData[3] << 8) | rawData[2];
      int16_t z_raw = ((int16_t)rawData[5] << 8) | rawData[4];

      float x_g = (float)x_raw / ADXL345_LSB_PER_G;
      float y_g = (float)y_raw / ADXL345_LSB_PER_G;
      float z_g = (float)z_raw / ADXL345_LSB_PER_G;

      // Gửi kết quả trở lại qua UART1
      Serial1.print("Accel:");
      Serial1.print(x_g, 2); Serial1.print(",");
      Serial1.print(y_g, 2); Serial1.print(",");
      Serial1.print(z_g, 2); Serial1.println("g");
      Serial.printf("[UARTtoSPI] Gửi gia tốc: X:%.2f, Y:%.2f, Z:%.2fg qua UART1\n", x_g, y_g, z_g);
    }
  }

  // Cập nhật LCD thường xuyên để hiển thị trạng thái
  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();
    byte rawData[6];
    readRegisters_UART(ADXL345_DATAX0, 6, rawData);

    int16_t x_raw = ((int16_t)rawData[1] << 8) | rawData[0];
    int16_t y_raw = ((int16_t)rawData[3] << 8) | rawData[2];
    int16_t z_raw = ((int16_t)rawData[5] << 8) | rawData[4];

    float x_g = (float)x_raw / ADXL345_LSB_PER_G;
    float y_g = (float)y_raw / ADXL345_LSB_PER_G;
    float z_g = (float)z_raw / ADXL345_LSB_PER_G;

    lcd.setCursor(0, 0);
    lcd.print("IN:UART  OUT:SPI");
    
    lcd.setCursor(0, 1);
    lcd.print("X:"); lcd.print(x_g, 1);
    lcd.print("Y:"); lcd.print(y_g, 1);
    lcd.print("Z:"); lcd.print(z_g, 1); 
    lcd.print("g");
  }
}