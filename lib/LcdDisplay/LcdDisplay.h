#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "TCA9548A.h"

class LcdDisplay {
public:
  LcdDisplay();
  void init();
  void clear();
  void setCursor(uint8_t col, uint8_t row);
  void print(const String& text);
  void print(const char* text);
  void print(int num);
  void print(float num);
  void print(uint32_t val);
  void print(uint8_t val);
  void print(char val);
  void print(const __FlashStringHelper* val);
  void printLine(uint8_t row, const String& text);
  void printStatus(const String& in, const String& out, uint32_t speed);
  void write(uint8_t value);

private:
  LiquidCrystal_I2C lcd;
  void select();  // chọn đúng channel trước khi ghi LCD
};

#endif
