#ifndef OS_SEMAPHORE
#define OS_SEMAPHORE

#include <stdint.h>

/**
 * Semaphore structure
 */

typedef struct {
    uint8_t count;
    uint8_t wait_list[NUMBER_OF_PROCESSES];
} t_os_semaphore;

void os_semaphore_init(t_os_semaphore *semaphore, uint8_t count);
int8_t os_semaphore_wait(t_os_semaphore *semaphore);
int8_t os_semaphore_signal(t_os_semaphore *semaphore);

#endif