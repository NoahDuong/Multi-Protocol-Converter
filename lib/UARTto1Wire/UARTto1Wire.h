#ifndef UARTTO1WIRE_H
#define UARTTO1WIRE_H

#include <stdint.h>
extern uint32_t globaluartbaudrate;
extern uint32_t onewirespeed;
void UARTto1Wire_setup();
void UARTto1Wire_loop();

#endif