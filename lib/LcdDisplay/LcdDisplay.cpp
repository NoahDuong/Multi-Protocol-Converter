#include <Arduino.h>
#include "LcdDisplay.h"

LcdDisplay::LcdDisplay() : lcd(0x27, 16, 2) {}

void LcdDisplay::select() {
  tcaselect(LCD_CHANNEL);  // Luôn chọn đúng channel trước khi giao tiếp LCD
}

void LcdDisplay::init() {
  select();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  Serial.println("[LCD] Init OK");
}

void LcdDisplay::clear() {
  select();
  lcd.clear();
}

void LcdDisplay::setCursor(uint8_t col, uint8_t row) {
  select();
  lcd.setCursor(col, row);
}

void LcdDisplay::print(const String& text) {
  select();
  lcd.print(text);
}

void LcdDisplay::print(const char* text) {
  select();
  lcd.print(text);
}

void LcdDisplay::print(int num) {
  select();
  lcd.print(num);
}

void LcdDisplay::print(float num) {
  select();
  lcd.print(num);
}

void LcdDisplay::print(uint32_t val) {
  select();
  lcd.print(val);
}

void LcdDisplay::print(uint8_t val) {
  select();
  lcd.print(val);
}

void LcdDisplay::print(char val) {
  select();
  lcd.print(val);
}

void LcdDisplay::print(const __FlashStringHelper* val) {
  select();
  lcd.print(val);
}

void LcdDisplay::printLine(uint8_t row, const String& text) {
  select();
  lcd.setCursor(0, row);
  lcd.print("                "); // Clear line
  lcd.setCursor(0, row);
  lcd.print(text);
}

void LcdDisplay::printStatus(const String& in, const String& out, uint32_t speed) {
  select();
  lcd.setCursor(0, 0);
  lcd.print("IN:" + in + "  OUT:" + out);
  lcd.setCursor(0, 1);
  lcd.print("SP:");
  lcd.print(speed / 1000);
  lcd.print("kHz");
}
void LcdDisplay::write(uint8_t value) {
  lcd.write(value);
}
