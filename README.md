# Bộ chuyển đổi đa giao thức trên ESP32 (Multi-Protocol Converter)

Dự án này biến một board mạch ESP32 thành một công cụ chuyển đổi giao thức linh hoạt, có khả năng chuyển đổi tín hiệu giữa các nguồn đầu vào (USB, UART) và các giao thức đầu ra phổ biến (UART, I2C, SPI, 1-Wire). Người dùng có thể thay đổi chế độ hoạt động và tốc độ truyền một cách nhanh chóng thông qua các nút nhấn vật lý, với trạng thái hiện tại được hiển thị trên màn hình LCD I2C.

## Tính năng nổi bật

- **Nguồn đầu vào linh hoạt**: Hỗ trợ nhận dữ liệu từ USB (qua Serial Monitor của máy tính) hoặc từ một cổng UART vật lý.
- **Đa dạng giao thức đầu ra**: Chuyển đổi và giao tiếp với các thiết bị sử dụng giao thức UART, I2C, SPI, và 1-Wire.
- **Cấu hình động**: Dễ dàng thay đổi nguồn vào, giao thức ra và tốc độ truyền mà không cần nạp lại firmware, chỉ bằng cách sử dụng các nút nhấn.
- **Hiển thị trạng thái trực quan**: Màn hình LCD 16x2 hiển thị rõ ràng chế độ đang hoạt động, bao gồm nguồn vào, giao thức ra và tốc độ tương ứng.
- **Cấu trúc module hóa**: Toàn bộ code được tổ chức thành các thư viện riêng biệt cho từng cặp chuyển đổi, giúp dễ dàng bảo trì và mở rộng.
- **Tích hợp cảm biến thực tế**: Dự án đi kèm với các trình điều khiển cho các cảm biến thông dụng để kiểm tra và minh họa từng giao thức:
  - **I2C**: Cảm biến ánh sáng BH1750.
  - **SPI**: Cảm biến gia tốc ADXL345.
  - **1-Wire**: Cảm biến nhiệt độ DS18B20.

## Phần cứng yêu cầu

- Board ESP32 (Dự án được phát triển trên ESP32 DEVKIT V1).
- Màn hình LCD 16x2 I2C.
- Mạch chuyển I2C Multiplexer TCA9548A (Do LCD và cảm biến BH1750 cùng sử dụng I2C).
- 4 x Nút nhấn (Push-buttons).
- Cảm biến để kiểm thử:
  - DS18B20 (1-Wire)
  - BH1750 (I2C)
  - ADXL345 (SPI)
- Breadboard và dây cắm.

## Phần mềm và Thư viện

Dự án được xây dựng trên nền tảng **PlatformIO**. Các thư viện cần thiết đã được khai báo trong file `platformio.ini`:

- `marcoschwartz/LiquidCrystal_I2C`
- `claws/BH1750`
- `paulstoffregen/OneWire`
- `milesburton/DallasTemperature`

## Sơ đồ kết nối (Pinout)

| Linh kiện              | Chân trên ESP32                                        | Ghi chú                                                              |
| ---------------------- | ------------------------------------------------------ | -------------------------------------------------------------------- |
| **Nút nhấn** |                                                        |                                                                      |
| - Chọn nguồn vào       | GPIO 32                                                | `BUTTON_INPUT_SOURCE`                                                |
| - Chọn giao thức ra    | GPIO 4                                                 | `BUTTON_OUTPUT_PROTOCOL`                                             |
| - Chọn tốc độ vào      | GPIO 15                                                | `BUTTON_INPUT_SPEED`                                                 |
| - Chọn tốc độ ra       | GPIO 2                                                 | `BUTTON_OUTPUT_SPEED`                                                |
| **Giao tiếp I2C** |                                                        | Kết nối vào mạch TCA9548A                                            |
| - SDA                  | GPIO 21                                                |                                                                      |
| - SCL                  | GPIO 22                                                |                                                                      |
| **Màn hình LCD I2C** | Kết nối vào Channel 0 của TCA9548A                     | `LCD_CHANNEL 0`                                                      |
| **Cảm biến BH1750** | Kết nối vào Channel 1 của TCA9548A                     | `BH1750_CHANNEL 1`                                                   |
| **Giao tiếp SPI** | (cho cảm biến ADXL345)                                 |                                                                      |
| - CS/SS                | GPIO 5                                                 |                                                                      |
| - SCK                  | GPIO 18                                                |                                                                      |
| - MISO                 | GPIO 19                                                |                                                                      |
| - MOSI                 | GPIO 23                                                |                                                                      |
| **Giao tiếp 1-Wire** | (cho cảm biến DS18B20)                                 |                                                                      |
| - Data Pin             | GPIO 25                                                |                                                                      |
| **Giao tiếp UART** |                                                        |                                                                      |
| - UART1 RX (Input)     | GPIO 27                                                |                                                                      |
| - UART1 TX (Input)     | GPIO 26                                                |                                                                      |
| - UART2 RX (Output)    | GPIO 16                                                | Dùng cho chế độ USB-to-UART và UART-to-UART                          |
| - UART2 TX (Output)    | GPIO 17                                                | Dùng cho chế độ USB-to-UART và UART-to-UART                          |

## Hướng dẫn sử dụng

1.  **Kết nối phần cứng**: Lắp ráp tất cả các linh kiện theo sơ đồ kết nối ở trên.
2.  **Cài đặt PlatformIO**: Mở dự án bằng VS Code đã cài đặt sẵn extension PlatformIO. PlatformIO sẽ tự động tải các thư viện cần thiết.
3.  **Biên dịch và Nạp code**: Biên dịch (Build) và Nạp (Upload) firmware vào board ESP32.
4.  **Vận hành**:
    - Mở **Serial Monitor** để xem các thông báo hệ thống và dữ liệu từ cảm biến.
    - Sử dụng các nút nhấn để cấu hình bộ chuyển đổi:
      - **Nút "Input Source"**: Chuyển đổi giữa nguồn vào là `USB` và `UART`.
      - **Nút "Output Protocol"**: Tuần tự chuyển đổi giao thức đầu ra giữa `UART`, `I2C`, `SPI`, và `1-Wire`.
      - **Nút "Input Speed"**: Thay đổi tốc độ Baud cho đầu vào UART.
      - **Nút "Output Speed"**: Thay đổi tốc độ/tần số cho giao thức đầu ra đang được chọn.
    - Màn hình LCD sẽ cập nhật và hiển thị ngay lập tức cấu hình bạn vừa chọn.

## Cấu trúc thư mục

-   `src/main.cpp`: File chính của chương trình, xử lý logic điều khiển, ngắt từ nút nhấn và luồng hoạt động chính.
-   `lib/`: Chứa các thư viện con, mỗi thư viện tương ứng với một chế độ chuyển đổi (ví dụ: `USBtoSPI`, `UARTtoI2C`,...). Điều này giúp mã nguồn trở nên gọn gàng và dễ quản lý.
-   `platformio.ini`: File cấu hình dự án cho PlatformIO, định nghĩa board, framework và các thư viện phụ thuộc.
