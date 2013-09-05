/**
 * OS
 *
 * Priority-based real-time multitasking operating system
 *
 * @author Jeff Stubler
 * @date March 10, 2012
 */

#include "os-main.h"
#include "os-core.h"

#ifdef __AVR_ATmega328P__
#else
#error Unsupported processor
#endif

typedef struct {
    char name[NAME_SIZE];
    uint32_t start_timestamp;
    uint16_t stack_pointer;
    uint8_t running;
    uint8_t delayed;
    uint8_t suspended;
    uint8_t semaphore_blocked;
} process_control_block;

static process_control_block pcb[NUMBER_OF_PROCESSES] __attribute__ ((section (".noinit")));

void os_init(void) __attribute__ ((naked, section (".init3")));
void os_init(void) {
    pcb[0].running = 1;
}

//int main(void) __attribute__ ((naked))
int main(void) {
    pcb[0].running = 0;
    return 0;
}

void os_stop(void) __attribute__ ((naked, noreturn, section (".fini0")));
void os_stop(void) {
    while (1);
}

/*
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
*/


