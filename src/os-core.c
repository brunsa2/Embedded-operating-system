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

#ifdef __AVR_ATmega328P__
#elif __AVR_TestEnv__
#else
#error Unsupported processor
#endif

#ifdef __AVR_TestEnv__
volatile unsigned long int stack_high, stack_low;
#define STACK_HIGH (stack_high)
#define STACK_LOW (stack_low)
#define PTR_TO_INT_CAST(ptr) ((unsigned long int) ptr)
#define SLEEP()
#elif __AVR_ATmega328P__
#define STACK_HIGH (*((volatile uint8_t *)(0x5e)))
#define STACK_LOW (*((volatile uint8_t *)(0x5d)))
#define PTR_TO_INT_CAST(ptr) ((uint16_t) ptr)
#define SLEEP() asm("sleep");
#endif

/* Context */

#ifdef __AVR_TestEnv__
#define SAVE_CONTEXT()
#define LOAD_CONTEXT()
#define RETURN()
#else
#define SAVE_CONTEXT() \
    asm volatile ( \
    "push r0 \n\t" \
    "in r0, __SREG__ \n\t" \
    "cli \n\t" \
    "push r0 \n\t" \
    "push r1 \n\t" \
    "clr r1 \n\t" \
    "push r2 \n\t" \
    "push r3 \n\t" \
    "push r4 \n\t" \
    "push r5 \n\t" \
    "push r6 \n\t" \
    "push r7 \n\t" \
    "push r8 \n\t" \
    "push r9 \n\t" \
    "push r10 \n\t" \
    "push r11 \n\t" \
    "push r12 \n\t" \
    "push r13 \n\t" \
    "push r14 \n\t" \
    "push r15 \n\t" \
    "push r16 \n\t" \
    "push r17 \n\t" \
    "push r18 \n\t" \
    "push r19 \n\t" \
    "push r20 \n\t" \
    "push r21 \n\t" \
    "push r22 \n\t" \
    "push r23 \n\t" \
    "push r24 \n\t" \
    "push r25 \n\t" \
    "push r26 \n\t" \
    "push r27 \n\t" \
    "push r28 \n\t" \
    "push r29 \n\t" \
    "push r30 \n\t" \
    "push r31 \n\t" \
    );
#define LOAD_CONTEXT() \
    asm volatile ( \
    "pop r31 \n\t" \
    "pop r30 \n\t" \
    "pop r29 \n\t" \
    "pop r28 \n\t" \
    "pop r27 \n\t" \
    "pop r26 \n\t" \
    "pop r25 \n\t" \
    "pop r24 \n\t" \
    "pop r23 \n\t" \
    "pop r22 \n\t" \
    "pop r21 \n\t" \
    "pop r20 \n\t" \
    "pop r19 \n\t" \
    "pop r18 \n\t" \
    "pop r17 \n\t" \
    "pop r16 \n\t" \
    "pop r15 \n\t" \
    "pop r14 \n\t" \
    "pop r13 \n\t" \
    "pop r12 \n\t" \
    "pop r11 \n\t" \
    "pop r10 \n\t" \
    "pop r9 \n\t" \
    "pop r8 \n\t" \
    "pop r7 \n\t" \
    "pop r6 \n\t" \
    "pop r5 \n\t" \
    "pop r4 \n\t" \
    "pop r3 \n\t" \
    "pop r2 \n\t" \
    "pop r1 \n\t" \
    "pop r0 \n\t" \
    "out __SREG__, r0 \n\t" \
    "pop r0 \n\t" \
    );
#define RETURN() asm volatile ("ret \n\t");
#endif

#ifdef __AVR_TestEnv__
#define TICK_ROLLOVER 0xf
#else
#define TICK_ROLLOVER 0x7fffffffL
#endif

volatile t_process_control_block pcb[NUMBER_OF_PROCESSES];
volatile uint8_t priority_buffer[NUMBER_OF_PROCESSES] NOINIT;
volatile uint8_t current_process;
volatile uint8_t idle_task_stack[IDLE_TASK_STACK_SIZE] NOINIT;
volatile uint16_t quantum_ticks;
volatile uint32_t system_ticks;
volatile uint8_t nesting_level;

static void os_idle_task(void) NORETURN;
static void os_terminate_current_task(void);
static void os_schedule(void);

void os_init(void) {
    uint8_t pcb_index;
    
    for (pcb_index = NUMBER_OF_PROCESSES - 1; pcb_index != 0; pcb_index--) {
        priority_buffer[pcb_index] = 0xff;
    }
    pcb[0].running = 1;
    pcb[0].stack_pointer = (uint8_t *) (STACK_HIGH << 8 | STACK_LOW);
    priority_buffer[NUMBER_OF_PROCESSES - 2] = 0;
        
    os_add_task(os_idle_task, &idle_task_stack[IDLE_TASK_STACK_SIZE - 1], NUMBER_OF_PROCESSES - 1);
    
#ifdef __AVR_ATmega328P__
	TCNT0 = 0;
	TCCR0A = (1 << WGM01); // CTC mode
    TCCR0B = (1 << CS01) | (1 << CS00); // clk/64
	OCR0A = 250; // clk/64/250 = clk/16000
	TIMSK0 |= (1 << OCIE0A);
#endif
}

void os_stop(void) NORETURN FINI1_SECTION NAKED;
void os_stop(void) {
    while (1);
}

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
    //new_task->terminate_function = os_terminate_current_task;
    //new_task->task_function = task;
    new_task->terminate_address_low = (uint8_t) (PTR_TO_INT_CAST(os_terminate_current_task) & 0xff);
    new_task->terminate_address_high = (uint8_t) (PTR_TO_INT_CAST(os_terminate_current_task) >> 8);
    new_task->task_address_low = (uint8_t) (PTR_TO_INT_CAST(task) & 0xff);
    new_task->task_address_high = (uint8_t) (PTR_TO_INT_CAST(task) >> 8);
    new_task->r1 = 0;
    new_task->sreg = 0x80;
    
	LEAVE_CRITICAL_SECTION();
    
	return current_pcb;
}

static void os_terminate_current_task(void) {
	os_remove_task(os_get_current_pid());
}

/**
 * Remove a task from running
 */
int8_t os_remove_task(uint8_t pid) {
	int current_priority;
	if (pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	ENTER_CRITICAL_SECTION();
	pcb[pid].running = 0;
	for (current_priority = 0; current_priority < NUMBER_OF_PROCESSES; current_priority++) {
		if (priority_buffer[current_priority] == pid) {
			priority_buffer[current_priority] = 0xff;
		}
	}
	LEAVE_CRITICAL_SECTION();
	os_switch_processes();
	return 0;
}

int8_t os_suspend_task(uint8_t pid) {
	if (pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	ENTER_CRITICAL_SECTION();
	pcb[pid].suspended = 1;
	LEAVE_CRITICAL_SECTION();
	os_switch_processes();
	return 0;
}

int8_t os_resume_task(uint8_t pid) {
	if (pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	ENTER_CRITICAL_SECTION();
	pcb[pid].suspended = 0;
	LEAVE_CRITICAL_SECTION();
    os_switch_processes();
	return 0;
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

/**
 * Start multitasking with the timer ticker
 */
void os_start(void) {
#ifndef __AVR_TestEnv__
	asm("sei");
#endif
}

// The lock/unlock pair should not contain any system calls that switch tasks
// (such as delays or semaphore signals). Use is undefined.
void os_lock_scheduler(void) {
    ENTER_CRITICAL_SECTION();
    if (nesting_level < 255) {
        nesting_level++;
    }
    LEAVE_CRITICAL_SECTION();
}

void os_unlock_scheduler(void) {
    ENTER_CRITICAL_SECTION();
    if (nesting_level > 0) {
        nesting_level--;
        if (!nesting_level) {
            LEAVE_CRITICAL_SECTION();
            os_switch_processes();
        } else {
            LEAVE_CRITICAL_SECTION();
        }
    } else {
        LEAVE_CRITICAL_SECTION();
    }
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
        os_switch_processes();
	}
}
    
void os_switch_processes(void) {
    SAVE_CONTEXT();
    
    pcb[current_process].stack_pointer = (uint8_t *) (STACK_HIGH << 8 | STACK_LOW);
    
    // TODO: Investigate round robbin of different tasks at priority level, ready list for quick context switcher
    os_schedule();
    
    STACK_HIGH = PTR_TO_INT_CAST(pcb[current_process].stack_pointer >> 8);
    STACK_LOW = PTR_TO_INT_CAST(pcb[current_process].stack_pointer & 0xff);
    
    LOAD_CONTEXT();
    RETURN();
}

// TODO: This may go into os_switch_processes
static void os_schedule(void) {
    int current_priority;
    if (nesting_level) {
        return;
    }
    for (current_priority = 0; current_priority < NUMBER_OF_PROCESSES; current_priority++) {
        uint8_t selected_pid = priority_buffer[current_priority];
        if (selected_pid == 0xff) {
            continue;
        } else {
            if (pcb[selected_pid].running == 1 && pcb[selected_pid].delayed == 0 && pcb[selected_pid].suspended == 0 && pcb[selected_pid].semaphore_blocked == 0) {
                current_process = selected_pid;
                break;
            } else if (pcb[selected_pid].delayed == 1 && system_ticks >= pcb[selected_pid].start_timestamp) {
                pcb[selected_pid].delayed = 0;
                current_process = selected_pid;
                break;
            }
        }
    }
}