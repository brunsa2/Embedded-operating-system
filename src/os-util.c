#include "os-util.h"

void copy_string(volatile char *destination, uint8_t destination_size, char *source) {
	volatile char *original_destination = destination;
	uint8_t original_destination_size = destination_size;
	while (*source != '\0' && destination_size > 0) {
		*destination = *source;
		destination++;
		source++;
		destination_size--;
	}
	original_destination[original_destination_size - 1] = '\0';
}