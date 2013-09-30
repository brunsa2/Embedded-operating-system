#ifndef __AVR_TestEnv__
#include "os-core.h"
#include "os-delay.h"
#include "os-semaphore.h"
#include "usart.h"

// This is a dummy main function for development purposes
volatile uint8_t test_stack[128] NOINIT;
volatile uint8_t test_2_stack[128] NOINIT;
t_os_semaphore a;

void test_task(void) {
    while (1) {
        uint8_t x;
        os_semaphore_wait(&a);
        for (x = 0; x < 3; x++) {
            usart_putsf("Hi World %d\r\n", x);
            os_delay(os_get_current_pid(), 1000);
        }
        os_semaphore_signal(&a);
        os_delay(os_get_current_pid(), 1000);
    }
}

void test_2_task(void) {
    while (1) {
        uint8_t x;
        os_semaphore_wait(&a);
        for (x = 20; x < 7; x++) {
            usart_putsf("G'day World %d\r\n", x);
            os_delay(os_get_current_pid(), 1000);
        }
        os_semaphore_signal(&a);
        os_delay(os_get_current_pid(), 1000);
    }
}

int main(void) {
    os_add_task(test_task, &test_stack[127], 1);
    os_add_task(test_2_task, &test_2_stack[127], 2);
    os_semaphore_init(&a, 1);
    usart_init(0, USART_TRANSMIT);
    os_start();
    
    while (1) {
        uint8_t x;
        os_semaphore_wait(&a);
        for (x = 10; x < 15; x++) {
            usart_putsf("Hello World %d\r\n", x);
            os_delay(os_get_current_pid(), 1000);
        }
        os_semaphore_signal(&a);
        os_delay(os_get_current_pid(), 1000);
    }
}
#endif