#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "os.h"
#include "usart.h"

#define STACK_HIGH (*((volatile uint8_t *)(0x5e)))
#define STACK_LOW (*((volatile uint8_t *)(0x5d)))

#define QUANTUM_MILLISECOND_LENGTH 10

typedef struct {
    uint16_t stack_pointer;
    uint16_t stack_start, stack_end;
    uint8_t running;
} process_control_block;

static process_control_block pcb[MAXIMUM_NUMBER_OF_PROCESSES];
static volatile uint8_t current_process;
static volatile uint16_t quantum_ticks;
static volatile uint32_t system_ticks;

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

static inline void schedule(void) {
    pcb[current_process].stack_pointer = STACK_HIGH << 8 | STACK_LOW;
    
	do {
        current_process = (current_process + 1) % MAXIMUM_NUMBER_OF_PROCESSES;
    } while (pcb[current_process].running == 0);
    
	STACK_HIGH = (uint8_t) (pcb[current_process].stack_pointer >> 8);
	STACK_LOW = (uint8_t) (pcb[current_process].stack_pointer & 0xff);
}

void switch_processes(void) __attribute__ ((naked, noinline));
void switch_processes(void) {
    SAVE_CONTEXT();
	schedule();
    LOAD_CONTEXT();
    RETURN();
}

uint8_t os_fork(volatile uint8_t *stack, uint16_t stack_size) {
    SAVE_CONTEXT();
    uint8_t new_pid = 0, old_pid = current_process;
    while (pcb[new_pid].running) {
        new_pid++;
    }
    pcb[new_pid].running = 1;
    pcb[new_pid].stack_start = (uint16_t) stack;
    pcb[new_pid].stack_end = (uint16_t) stack + stack_size - 1;
    usart_putsf("Old stack from 0x%y  to 0x%y  \r\n", pcb[0].stack_start, pcb[0].stack_end);
    usart_putsf("New stack from 0x%y  to 0x%y  \r\n", pcb[new_pid].stack_start, pcb[new_pid].stack_end);
    uint16_t stack_size_used = pcb[current_process].stack_end - ((STACK_HIGH << 8) | STACK_LOW);
    usart_putsf("Stack used %d\r\n", stack_size_used);
    pcb[new_pid].stack_pointer = pcb[new_pid].stack_end - stack_size_used;
    uint8_t *source = (uint8_t *) ((STACK_HIGH << 8) | STACK_LOW);
    uint8_t *destination = (uint8_t *) pcb[new_pid].stack_pointer;
    usart_putsf("Source to destination 0x%y  to 0x%y  \r\n", (uint16_t) source, (uint16_t) destination);
    while ((uint16_t) source < pcb[current_process].stack_end + 1) {
        //*destination++ = *source++;
    }
    LOAD_CONTEXT();
    if (current_process == old_pid) {
        return new_pid;
    } else {
        return 0;
    }
}

static volatile uint8_t new_task_stack[128] __attribute__ ((aligned(16)));

int main(void) {
    uint8_t pcb_index;
	for (pcb_index = 0; pcb_index < MAXIMUM_NUMBER_OF_PROCESSES; pcb_index++) {
		pcb[pcb_index].running = 0;
	}
    
	pcb[0].running = 1;
    pcb[0].stack_start = (RAMEND - INIT_TASK_STACK_SIZE + 1);
    pcb[0].stack_end = RAMEND;
    current_process = 0;
    
#ifdef __AVR_ATmega328P__
	TCNT0 = 0;
	TCCR0A = (1 << WGM01); // CTC mode
    TCCR0B = (1 << CS01) | (1 << CS00); // clk/64
	OCR0A = 250; // clk/64/250 = clk/16000
	TIMSK0 |= (1 << OCIE0A);
#else
#error Unsupported processor
#endif
    
    asm volatile ("sei");
    
    usart_init(0, USART_TRANSMIT);
    
    uint8_t pid = os_fork(new_task_stack, 128);
    
    if (current_process) {
        while (1) {
            //usart_putsf("*\r\n");
        }
    } else {
        while (1) {
            //usart_putsf("!\r\n");
        }
    }
    
    return 0;
}

#ifdef __AVR_ATmega328P__
ISR(TIMER0_COMPA_vect) {
#else
#error Unsupported processor
#endif
	quantum_ticks++;
	system_ticks++;
    
	if (quantum_ticks >= QUANTUM_MILLISECOND_LENGTH) {
		quantum_ticks = 0;
        usart_putsf("*");
        switch_processes();
	}
}
