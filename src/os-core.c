/**
 * OS
 *
 * Priority-based real-time multitasking operating system
 *
 * @author Jeff Stubler
 * @date March 10, 2012
 */

#ifdef __AVR_TestEnv__
#include "../test/os-conf.h"
#include "../test/avr/io.h"
#else
#include "os-conf.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#endif
#include "os-core.h"
#include "os-util.h"

#ifdef __AVR_ATmega328P__
#elif __AVR_TestEnv__
#else
#error Unsupported processor
#endif

#ifdef __AVR_TestEnv__
volatile unsigned long int stack_high, stack_low;
#define STACK_HIGH (stack_high)
#define STACK_LOW (stack_low)
#define SLEEP()
#elif __AVR_ATmega328P__
#define STACK_HIGH (*((volatile uint8_t *)(0x5e)))
#define STACK_LOW (*((volatile uint8_t *)(0x5d)))
#define SLEEP() asm("sleep");
#endif

/* Critical sections */

#ifdef __AVR_TestEnv__
#define ENTER_CRITICAL_SECTION()
#define ENTER_CRITICAL_SECITON_AGAIN()
#define LEAVE_CRITICAL_SECTION()
#elif __AVR_ATmega328P__
#define ENTER_CRITICAL_SECTION() \
uint8_t flags = SREG; \
asm volatile ("cli");

#define ENTER_CRITICAL_SECTION_AGAIN() \
flags = SREG; \
asm volatile ("cli");

#define LEAVE_CRITICAL_SECTION() \
SREG = flags;
#endif

#ifdef __AVR_TestEnv__
#define TICK_ROLLOVER 0xf
#else
#define TICK_ROLLOVER 0x7fffffff
#endif

volatile t_process_control_block pcb[NUMBER_OF_PROCESSES];
static volatile uint8_t priority_buffer[NUMBER_OF_PROCESSES] NOINIT;
volatile uint8_t current_process;
volatile uint8_t idle_task_stack[IDLE_TASK_STACK_SIZE] NOINIT;
volatile uint16_t quantum_ticks;
volatile uint32_t system_ticks;

static void os_idle_task(void) NORETURN;
static void os_terminate_current_task(void);

void os_init(void) {
    uint8_t pcb_index;
    
    for (pcb_index = NUMBER_OF_PROCESSES - 1; pcb_index != 0; pcb_index--) {
        priority_buffer[pcb_index] = 0xff;
    }
    pcb[0].running = 1;
    pcb[0].stack_pointer = (uint8_t *) (STACK_HIGH << 8 | STACK_LOW);
    priority_buffer[NUMBER_OF_PROCESSES - 2] = 0;
        
    os_add_task(os_idle_task, &idle_task_stack[IDLE_TASK_STACK_SIZE - 1], NUMBER_OF_PROCESSES - 1);
    //enable_timer();
}

void os_stop(void) NORETURN FINI1_SECTION NAKED;
void os_stop(void) {
    while (1);
}

#ifndef __AVR_TestEnv__
// This is a dummy main function for development purposes
int main(void) {
    return 0;
}
#endif

int8_t os_add_task(void (*task)(void), volatile uint8_t *stack, uint8_t priority) {
	if (priority >= NUMBER_OF_PROCESSES) {
		return -1;
	}
    
	ENTER_CRITICAL_SECTION();
    
	uint8_t current_pcb = 0;
    
	while (pcb[current_pcb].running == 1) {
		current_pcb++;
	}
    
	if (priority_buffer[priority] != 0xff || current_pcb >= NUMBER_OF_PROCESSES) {
		LEAVE_CRITICAL_SECTION();
		return -1;
	}
    
	pcb[current_pcb].running = 1;
	pcb[current_pcb].stack_pointer = stack - sizeof(t_task_stack_frame);
	priority_buffer[priority] = current_pcb;
    
    t_task_stack_frame *new_task = (t_task_stack_frame *) (stack - sizeof(t_task_stack_frame) + 1);
    new_task->terminate_function = os_terminate_current_task;
    new_task->task_function = task;
    new_task->r1 = 0;
    new_task->sreg = 0x80;
    
	LEAVE_CRITICAL_SECTION();
    
	return current_pcb;
}

static void os_terminate_current_task(void) {
	
}

static void os_idle_task(void) {
	// Enable idle mode
	MCUCR |= (1 << SE);
	MCUCR &= 0xff - ((1 << SM2) | (1 << SM1) | (1 << SM0));
	while (1) {
		SLEEP();
	}
}

uint8_t os_get_current_pid(void) {
	return current_process;
}

#ifdef __AVR_TestEnv__
void timer_interrupt(void) {
#elif __AVR_ATmega328P__
ISR(TIMER0_COMPA_vect) {
#endif
	quantum_ticks++;
	system_ticks++;
    
	if (quantum_ticks >= QUANTUM_MILLISECOND_LENGTH) {
         if (system_ticks >= TICK_ROLLOVER) {
            // Adjust timestamps so that timestamps never roll over
            uint8_t pid;
            for (pid = 0; pid < NUMBER_OF_PROCESSES; pid++) {
                if (pcb[pid].delayed) {
                    pcb[pid].start_timestamp -= system_ticks + 1;
                }
            }
            system_ticks = 0;
        }
		quantum_ticks = 0;
        //os_switch_processes();
	}
}