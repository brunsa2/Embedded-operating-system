#include "os.h"
#include "usart.h"

#include <stdint.h>

#define STACK_SIZE 64

extern volatile os_stack_free_list_node *stack_free_list;

volatile uint8_t test_stack[STACK_SIZE + 64];
volatile uint8_t test_2_stack[STACK_SIZE + 64];

void test_task();
void test_2_task();

void test_task() {
	while (1) {
        usart_puts("Hello World!\r\n");
        os_delay(os_get_current_pid(), 1000);
    }
}

void test_2_task() {
    usart_puts("Hi!\r\n");
    os_delay(os_get_current_pid(), 5000);
    usart_puts("Hi 2!\r\n");
    usart_putsf("Stack free list at 0x%y\r\n", (uint16_t) stack_free_list);
}

INIT() {
    os_add_task(test_task, &test_stack[STACK_SIZE + 63], 1, "test");
    os_add_task(test_2_task, &test_2_stack[STACK_SIZE + 63], 2, "tst2");
    usart_init(0, USART_TRANSMIT);
    os_start_ticker();
    
    while(1) {
        
        
    }
    
    //return 0;
}
