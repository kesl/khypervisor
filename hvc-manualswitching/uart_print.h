#ifndef __UART_PRINT_H__
#define __UART_PRINT_H__
void uart_putc( char c );
void uart_print(char *str);
void uart_print_hex32( unsigned int v );

#endif
