#ifndef UARTTOI2C_H
#define UARTTOI2C_H

#include <stdint.h>
extern uint32_t globaluartbaudrate;
extern uint32_t globali2cFrequency;
void UARTtoI2C_setup();
void UARTtoI2C_loop();

#endif