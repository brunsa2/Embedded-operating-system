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
#ifdef __AVR_TestEnv__
#include "../test/os-conf.h"
#else
#include "os-conf.h"
#endif
//#include <avr/interrupt.h>
//#include <avr/io.h>

#ifdef __AVR_ATmega328P__
#define NAKED __attribute__ ((naked))
#define NOINIT __attribute__ ((section (".noinit")))
#define INIT5_SECTION __attribute__ ((section (".init5")))
#define FINI1_SECTION __attribute__ ((section (".fini0")))
#define NORETURN __attribute__ ((noreturn))
#elif __AVR_TestEnv__
#define NAKED
#define NOINIT
#define INIT5_SECTION
#define FINI1_SECTION
#define NORETURN
#else
#error Unsupported processor
#endif

// TODO: Add preprocess check on various params

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

/* Init task */

#define INIT() \
void init_task(void) __attribute__ ((noreturn)); \
void init_task(void);

typedef struct {
    uint8_t r31, r30, r29, r28, r27, r26, r25, r24;
    uint8_t r23, r22, r21, r20, r19, r18, r17, r16;
    uint8_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint8_t r7, r6, r5, r4, r3, r2, r1, sreg, r0;
    uint8_t task_address_high, task_address_low;
    uint8_t terminate_address_high, terminate_address_low;
    void (*task_function)(void);
    void (*terminate_function)(void);
} t_task_stack_frame;

typedef struct {
    uint32_t start_timestamp;
    // TODO: Rename to stack
    volatile uint8_t *stack_pointer;
    uint8_t running : 1;
    uint8_t delayed : 1;
    uint8_t suspended : 1;
    uint8_t semaphore_blocked : 1;
} t_process_control_block;

#ifdef __AVR_TestEnv__
void timer_interrupt(void);
#endif

/**
 * Initialize operating system
 */
//void os_init(void);
void os_init(void) INIT5_SECTION NAKED;

/**
 * Start multitasking with the timer ticker
 */
//void os_start_ticker(void);

/**
 * Add new task to operating system
 */
int8_t os_add_task(void (*task)(void), volatile uint8_t *stack, uint8_t priority);

/**
 * Remove a task from the operating system
 */
//int8_t os_remove_task(uint8_t pid);

/**
 * Get current task
 */
uint8_t os_get_current_pid(void);

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
