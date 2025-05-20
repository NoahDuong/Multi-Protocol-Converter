#ifndef USBTOI2C_H
#define USBTOI2C_H
#include <stdint.h>

extern uint32_t globali2cFrequency;
void USBtoI2C_setup();
void USBtoI2C_loop();

#endif