#ifndef TCA9548A_H
#define TCA9548A_H

#include <Wire.h>

#define TCA_ADDR 0x70         // Địa chỉ I2C mặc định của TCA9548A
#define LCD_CHANNEL 0         // Channel đang gắn LCD (sửa nếu bạn gắn kênh khác)

inline void tcaselect(uint8_t channel) {
  if (channel > 7) return;    // Chỉ hỗ trợ channel 0-7
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);   // Chọn channel
  Wire.endTransmission();
}

#endif
