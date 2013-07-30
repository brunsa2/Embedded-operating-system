/**
 * OS
 *
 * Priority-based real-time multitasking operating system
 *
 * @author Jeff Stubler
 * @date March 10, 2012
 */

#include "os.h"
#include "usart.h"

#ifdef __AVR_ATmega328P__
#else
#error Unsupported processor
#endif

#ifdef __AVR_ATmega328P__
#define STACK_HIGH (*((volatile uint8_t *)(0x5e)))
#define STACK_LOW (*((volatile uint8_t *)(0x5d)))
#endif

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

#define HALF_TICKS 0x7fffffff

void init_task(void);

typedef struct {
    char name[NAME_SIZE];
    uint32_t start_timestamp;
    uint16_t stack_pointer;
    uint8_t running;
    uint8_t delayed;
    uint8_t suspended;
    uint8_t semaphore_blocked;
} process_control_block;

typedef struct {
    uint8_t r31, r30, r29, r28, r27, r26, r25, r24;
    uint8_t r23, r22, r21, r20, r19, r18, r17, r16;
    uint8_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint8_t r7, r6, r5, r4, r3, r2, r1, sreg, r0;
    uint8_t task_address_high, task_address_low;
    uint8_t terminate_address_high, terminate_address_low;
} t_task_stack_frame;

static volatile uint8_t priority_buffer[NUMBER_OF_PROCESSES];
static process_control_block pcb[NUMBER_OF_PROCESSES];
static volatile uint8_t current_process;
static volatile uint16_t quantum_ticks = 0;
static volatile uint32_t system_ticks = 0x100000000 - 10000;
static volatile uint8_t idle_task_stack[IDLE_TASK_STACK_SIZE];
static volatile uint8_t nesting_level = 0;

volatile os_stack_free_list_node *stack_free_list;

static uint8_t *os_get_free_stack(uint8_t blocks) {
    return (uint8_t *) NULL;
}

static uint8_t *os_find_exact_or_biggest_stack_space(uint8_t blocks) {
    os_stack_free_list_node *node = stack_free_list;
    uint8_t largest_size = 0;
    os_stack_free_list_node *largest_node = NULL;
    while (node->next != OS_FREE_LIST_NULL) {
        if (blocks == node->size) {
            return (uint8_t*) (node + STACK_BLOCK_SIZE - 1);
        }
        if (node->size > largest_size) {
            largest_size = node->size;
            largest_node = node;
        }
    }
    return largest_node;
}

static void os_terminate_current_task(void) {
	os_remove_task(os_get_current_pid());
}

static void os_idle_task() {
	// Enable idle mode
	MCUCR |= (1 << SE);
	MCUCR &= 0xff - ((1 << SM2) | (1 << SM1) | (1 << SM0));
	while (1) {
		asm("sleep");
	}
}

void enable_timer(void) {
#ifdef __AVR_ATmega328P__
	TCNT0 = 0;
	TCCR0A = (1 << WGM01); // CTC mode
    TCCR0B = (1 << CS01) | (1 << CS00); // clk/64
	OCR0A = 250; // clk/64/250 = clk/16000
	TIMSK0 |= (1 << OCIE0A);
#endif
}

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

void os_switch_processes(void) __attribute__ ((naked, noinline));
void os_switch_processes(void) {
	SAVE_CONTEXT();

	pcb[current_process].stack_pointer = STACK_HIGH << 8 | STACK_LOW;
    
	// TODO: Investigate round robbin of different tasks at priority level, ready list for quick context switcher
	os_schedule();

	STACK_HIGH = (uint8_t) (pcb[current_process].stack_pointer >> 8);
	STACK_LOW = (uint8_t) (pcb[current_process].stack_pointer & 0xff);

	LOAD_CONTEXT();
    RETURN();
}

/**
 * Initialize operating system
 */
//void os_init(void) {
int main(void) __attribute__ ((noreturn));
int main(void) {
    stack_free_list = (os_stack_free_list_node *) (RAMEND - (INIT_STACK_BLOCKS * STACK_BLOCK_SIZE) + 1);
    stack_free_list->next = OS_FREE_LIST_NULL;
    stack_free_list->size = STACK_BLOCKS - INIT_STACK_BLOCKS;
    
    uint8_t pcb_index;
	for (pcb_index = 0; pcb_index < NUMBER_OF_PROCESSES; pcb_index++) {
		pcb[pcb_index].running = 0;
        pcb[pcb_index].delayed = 0;
        pcb[pcb_index].suspended = 0;
        pcb[pcb_index].semaphore_blocked = 0;
		copy_string(pcb[pcb_index].name, NAME_SIZE, "");
		pcb[pcb_index].stack_pointer = 0;
		priority_buffer[pcb_index] = 0xff;
	}

	pcb[0].running = 1;
	copy_string(pcb[0].name, NAME_SIZE, "init");
	pcb[0].stack_pointer = STACK_HIGH << 8 | STACK_LOW;
	priority_buffer[NUMBER_OF_PROCESSES - 2] = 0;

	current_process = 0;
    
    usart_init(0, USART_TRANSMIT);
    
	os_add_task(os_idle_task, &idle_task_stack[IDLE_TASK_STACK_SIZE - 1], NUMBER_OF_PROCESSES - 1, "idle");
	enable_timer();
    
    asm volatile ("ijmp" : : "z" ((uint16_t) init_task));
    while (1);
}

/**
 * Start multitasking with the timer ticker
 */
void os_start_ticker(void) {
	asm("sei");
}

/**
 * Delay task for specified number of ticks
 *
 * @param pid Process ID to delay
 * @param ticks Number of ticks to delay
 */
int8_t os_delay(uint8_t pid, uint32_t ticks) {
	if (pid < 0 || pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	uint32_t start_timestamp = system_ticks + ticks;
	ENTER_CRITICAL_SECTION();
	pcb[pid].start_timestamp = start_timestamp;
    pcb[pid].delayed = 1;
	LEAVE_CRITICAL_SECTION();
	os_switch_processes();
	return 0;
}

/**
 * Cancel any delay on a task
 * @param pid Process ID to cancel delay for
 */
int8_t os_cancel_delay(uint8_t pid) {
    if (pid < 0 || pid >= NUMBER_OF_PROCESSES) {
        return -1;
    }
    ENTER_CRITICAL_SECTION();
    pcb[pid].delayed = 0;
    LEAVE_CRITICAL_SECTION();
    os_switch_processes();
    return 0;
}

/**
 * Add new task to operating system
 */
int8_t os_add_task(void (*task)(void), volatile uint8_t *stack, uint8_t priority, char *name) {
	if (priority < 0 || priority >= NUMBER_OF_PROCESSES) {
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
	copy_string(pcb[current_pcb].name, NAME_SIZE, name);
	pcb[current_pcb].stack_pointer = (uint16_t) stack - sizeof(t_task_stack_frame);
	priority_buffer[priority] = current_pcb;
    
    t_task_stack_frame *new_task = (t_task_stack_frame *) (stack - sizeof(t_task_stack_frame) + 1);
    new_task->terminate_address_low = (uint8_t) ((uint16_t) os_terminate_current_task & 0xff);
    new_task->terminate_address_high = (uint8_t) ((uint16_t) os_terminate_current_task >> 8);
    new_task->task_address_low = (uint8_t) ((uint16_t) task & 0xff);
    new_task->task_address_high = (uint8_t) ((uint16_t) task >> 8);
    new_task->sreg = 0x80;

	LEAVE_CRITICAL_SECTION();

	return current_pcb;
}

/**
 * Remove a task from running
 */
int8_t os_remove_task(uint8_t pid) {
	int current_priority;
	if (pid < 0 || pid >= NUMBER_OF_PROCESSES) {
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

/**
 * Get current task
 */
uint8_t os_get_current_pid(void) {
	return current_process;
}

// TODO: Add destination buffer size and do not overwrite it
void copy_string(char *destination, uint8_t destination_size, char *source) {
	char *original_destination = destination;
	uint8_t original_destination_size = destination_size;
	while (*source != '\0' && destination_size > 0) {
		*destination = *source;
		destination++;
		source++;
		destination_size--;
	}
	original_destination[original_destination_size - 1] = '\0';
}

void os_set_task_name(uint8_t pid, char *name) {
	copy_string(pcb[pid].name, NAME_SIZE, name);
}

char *os_get_task_name(uint8_t pid) {
	return pcb[pid].name;
}

int8_t os_set_task_priority(uint8_t pid, uint8_t priority) {
	if (priority < 0 || priority >= NUMBER_OF_PROCESSES || pid < 0 || pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	ENTER_CRITICAL_SECTION();
	uint8_t old_priority = os_get_task_priority(pid);
	if (priority_buffer[priority] == 0xff) {
		priority_buffer[priority] = pid;
		priority_buffer[old_priority] = 0xff;
	}
	LEAVE_CRITICAL_SECTION();
	os_switch_processes();
	return 0;
}

int8_t os_get_task_priority(uint8_t pid) {
	int current_priority;
	for (current_priority = 0; current_priority < NUMBER_OF_PROCESSES; current_priority++) {
		if (priority_buffer[current_priority] == pid) {
			return current_priority;
		}
	}
	return -1;
}

int8_t os_suspend_task(uint8_t pid) {
	if (pid < 0 || pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	ENTER_CRITICAL_SECTION();
	pcb[pid].suspended = 1;
	LEAVE_CRITICAL_SECTION();
	os_switch_processes();
	return 0;
}

int8_t os_resume_task(uint8_t pid) {
	if (pid < 0 || pid >= NUMBER_OF_PROCESSES) {
		return -1;
	}
	ENTER_CRITICAL_SECTION();
	pcb[pid].suspended = 0;
	LEAVE_CRITICAL_SECTION();
	return 0;
}

void os_semaphore_init(os_semaphore *semaphore, uint8_t count) {
    semaphore->count = count;
    uint8_t pid;
    for (pid = 0; pid < NUMBER_OF_PROCESSES; pid++) {
        semaphore->wait_list[pid] = 0;
    }
}

int8_t os_semaphore_wait(os_semaphore *semaphore) {
    ENTER_CRITICAL_SECTION();
    if (semaphore->count > 0) {
        semaphore->count--;
        LEAVE_CRITICAL_SECTION();
        return 0;
    }
    uint8_t pid = os_get_current_pid();
    semaphore->wait_list[pid] = 1;
    pcb[pid].semaphore_blocked = 1;
    LEAVE_CRITICAL_SECTION();
    os_switch_processes();
    ENTER_CRITICAL_SECTION_AGAIN();
    semaphore->count--;
    LEAVE_CRITICAL_SECTION();
    return 0;
}

int8_t os_semaphore_signal(os_semaphore *semaphore) {
    ENTER_CRITICAL_SECTION();
    if (semaphore->count == 0) {
        uint8_t pid;
        for (pid = 0; pid < NUMBER_OF_PROCESSES; pid++) {
            pcb[pid].semaphore_blocked = 0;
            semaphore->wait_list[pid] = 0;
        }
        LEAVE_CRITICAL_SECTION();
        os_switch_processes();
        ENTER_CRITICAL_SECTION_AGAIN();
    }
    if (semaphore->count < 255) {
        semaphore->count++;
        LEAVE_CRITICAL_SECTION();
        return 0;
    }
    LEAVE_CRITICAL_SECTION();
    return -1;
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

#ifdef __AVR_ATmega328P__
ISR(TIMER0_COMPA_vect) {
#endif
	quantum_ticks++;
	system_ticks++;

	if (quantum_ticks >= QUANTUM_MILLISECOND_LENGTH) {
        if (system_ticks > HALF_TICKS) {
            // Adjust timestamps so that timestamps never roll over
            system_ticks -= HALF_TICKS;
            uint8_t pid;
            for (pid = 0; pid < NUMBER_OF_PROCESSES; pid++) {
                if (pcb[pid].delayed) {
                    pcb[pid].start_timestamp -= HALF_TICKS;
                }
            }
        }
		quantum_ticks = 0;
        os_switch_processes();
	}
}
