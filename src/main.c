#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include "usart.h"

int main() {
    usart_init(0, USART_TRANSMIT | USART_RECEIVE);
    usart_putsf("Hello World\r\n");
    while (1) {
        usart_putsf("Hello World\r\n");
    }
    return 0;
}