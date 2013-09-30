#ifndef OS_DELAY
#define OS_DELAY

#include <stdint.h>

int8_t os_delay(uint8_t pid, uint32_t ticks);
int8_t os_cancel_delay(uint8_t pid);

#endif