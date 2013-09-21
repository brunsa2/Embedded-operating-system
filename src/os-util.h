#ifndef OS_UTIL
#define OS_UTIL

#include <stdint.h>

void copy_string(volatile char *destination, uint8_t destination_size, char *source);

#endif