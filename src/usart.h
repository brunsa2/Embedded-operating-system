/**
 * USART driver
 *
 * Enables asynchronous serial communication, both receiving and transmitting, at single-character level
 *
 * @author Jeff Stubler
 * @date March 9, 2012
 */

#ifndef USART
#define USART

#include <avr/io.h>
#include <stdint.h>

#define USART_BAUD 115200L

/**
 * Enable transmitting
 */
#define USART_TRANSMIT 0x01

/**
 * Enable receiving
 */
#define USART_RECEIVE 0x02

/**
 * Initialize USART with specified baud rate and options
 * @param baud Baud rate
 * @param flags Flags for options for serial port
 * @return Nothing
 */
void usart_init(uint32_t baud, uint8_t flags);

/**
 * Send one byte over USART
 * @param data Data byte to transmit
 */
int usart_putc(char data);

/**
 * Send string over USART
 * @param string String to send
 */
void usart_puts(char *string);

/**
 * Receive one byte over USART
 * @return Byte from USART
 */
int usart_getc(void);

/**
 * Check if the USART has received a character
 * @return Number of available characters from the USART
 */
int usart_hasc(void);

/**
 * Send formatted string over USART
 * @param string Formatted string to send
 * @param ... Items to send
 */
void usart_putsf(const char *string, ...);

#endif
