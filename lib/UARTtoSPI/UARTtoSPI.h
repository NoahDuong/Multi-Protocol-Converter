#ifndef UARTTOSPI_H
#define UARTTOSPI_H

#include <stdint.h>

extern uint32_t globaluartbaudrate;
extern uint32_t globalspiFrequency;
void UARTtoSPI_setup();
void UARTtoSPI_loop();

#endif