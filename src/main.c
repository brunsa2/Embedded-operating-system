#ifndef __AVR_TestEnv__
#include "os-core.h"
#include "os-delay.h"
#include "usart.h"

// This is a dummy main function for development purposes
volatile uint8_t test_stack[128] NOINIT;
extern volatile uint32_t system_ticks;

void test_task(void) {
    while (1) {
        usart_puts("!");
        os_delay(os_get_current_pid(), 1000);
    }
}

int main(void) {
    os_add_task(test_task, &test_stack[127], 1);
    usart_init(0, USART_TRANSMIT);
    os_start();
    
    while (1) {
        usart_putsf("Hello World\r\n");
        os_delay(os_get_current_pid(), 1000);
    }
}
#endif