#ifndef __AVR_TestEnv__
#include "os-core.h"
#include "os-delay.h"
#include "usart.h"

// This is a dummy main function for development purposes
volatile uint8_t test_stack[128] NOINIT;
volatile uint8_t test_2_stack[128] NOINIT;
extern volatile uint32_t system_ticks;
static volatile uint8_t pid;

void test_task(void) {
    pid = os_get_current_pid();
    while (1) {
        usart_puts("!");
        os_delay(os_get_current_pid(), 1000);
    }
}

void test_2_task(void) {
    while (1) {
        os_cancel_delay(pid);
    }
}

int main(void) {
    os_add_task(test_task, &test_stack[127], 1);
    os_add_task(test_2_task, &test_2_stack[127], 2);
    usart_init(0, USART_TRANSMIT);
    os_start();
    
    while (1) {
        usart_putsf("Hello World\r\n");
        os_delay(os_get_current_pid(), 1000);
    }
}
#endif