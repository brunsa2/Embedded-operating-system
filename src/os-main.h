/**
 * OS
 *
 * Priority-based soft real-time multitasking operating system
 *
 * @author Jeff Stubler
 * @date March 10, 2012
 */

#ifndef OS
#define OS

#include <inttypes.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>

// TODO: Add preprocess check on various params
/**
 * Maximum number of processes that can be managed
 */
#define NUMBER_OF_PROCESSES 6

/**
 * Length of each time quantum (ms)
 */
#define QUANTUM_MILLISECOND_LENGTH 10

#define NAME_SIZE 5

/**
 * Stack size
 */
#define IDLE_TASK_STACK_SIZE 64

#define STACK_BLOCK_SIZE 32
#define STACK_BLOCKS 16
#define INIT_STACK_BLOCKS 4

/**
 * Semaphore structure
 */

typedef struct {
    uint8_t count;
    uint8_t wait_list[NUMBER_OF_PROCESSES];
} os_semaphore;

typedef struct {
    uint8_t next, size;
} os_stack_free_list_node;

#define OS_FREE_LIST_NULL 255

/* Critical sections */

#define ENTER_CRITICAL_SECTION() \
	uint8_t flags = SREG; \
	asm volatile ("cli");

#define ENTER_CRITICAL_SECTION_AGAIN() \
    flags = SREG; \
    asm volatile ("cli");
	
#define LEAVE_CRITICAL_SECTION() \
	SREG = flags;

/* Init task */

#define INIT() \
    void init_task(void) __attribute__ ((noreturn)); \
    void init_task(void)

/**
 * Initialize operating system
 */
//void os_init(void);

/**
 * Start multitasking with the timer ticker
 */
//void os_start_ticker(void);

/**
 * Add new task to operating system
 */
//int8_t os_add_task(void (*task)(void), volatile uint8_t *stack, uint8_t priority, char *name);

/**
 * Remove a task from the operating system
 */
//int8_t os_remove_task(uint8_t pid);

/**
 * Get current task
 */
//uint8_t os_get_current_pid(void);

//void copy_string(char *destination, uint8_t destination_size, char *source);

/**
 * Delay task for specified number of ticks
 *
 * @param pid Process ID to delay
 * @param ticks Number of ticks to delay
 */
//int8_t os_delay(uint8_t pid, uint32_t ticks);

/**
 * Cancel any delay on a task
 * @param pid Process ID to cancel delay for
 * @return Error code
 */
//int8_t os_cancel_delay(uint8_t pid);

//void os_set_task_name(uint8_t pid, char *name);
//char *os_get_task_name(uint8_t pid);

//int8_t os_set_task_priority(uint8_t pid, uint8_t priority);
//int8_t os_get_task_priority(uint8_t pid);

//int8_t os_suspend_task(uint8_t pid);
//int8_t os_resume_task(uint8_t pid);

//void os_semaphore_init(os_semaphore *semaphore, uint8_t count);
//int8_t os_semaphore_wait(os_semaphore *semaphore);
//int8_t os_semaphore_signal(os_semaphore *semaphore);

//void os_lock_scheduler(void);
//void os_unlock_scheduler(void);

#endif
