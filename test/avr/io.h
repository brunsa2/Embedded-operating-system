#ifndef AVR_IO
#define AVR_IO

#include <stdint.h>

static volatile uint8_t mcucr;
#define MCUCR (mcucr)
#define SE 0
#define SM0 1
#define SM1 2
#define SM2 3

#endif