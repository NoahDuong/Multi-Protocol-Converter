#ifndef USBTOSPI_H
#define USBTOSPI_H

#include <stdint.h>

extern uint32_t globalspiFrequency;
void USBtoSPI_setup();
void USBtoSPI_loop();

#endif