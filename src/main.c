#ifndef __AVR_TestEnv__
#include "os-core.h"
#include "os-delay.h"
#include "usart.h"

// This is a dummy main function for development purposes
volatile uint8_t test_stack[128] NOINIT;
static volatile uint8_t pid;

void test_task(void) {
    uint8_t x;
    pid = os_get_current_pid();
    for (x = 0; x < 5; x++) {
        usart_putsf("!\r\n");
        os_delay(os_get_current_pid(), 1000);
    }
}

int main(void) {
    os_add_task(test_task, &test_stack[127], 1);
    usart_init(0, USART_TRANSMIT);
    os_start();
    
    uint8_t x;
    for (x = 0; x < 3; x++) {
        usart_putsf("Hello World\r\n");
        os_delay(os_get_current_pid(), 1000);
    }
    
    os_remove_task(pid);
    
    while (1) {
        usart_putsf("Hello World\r\n");
        os_delay(os_get_current_pid(), 1000);
    }
}
#endif