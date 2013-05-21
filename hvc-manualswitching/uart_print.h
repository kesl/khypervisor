#ifndef __UART_PRINT_H__
#define __UART_PRINT_H__
#include "arch_types.h"

void uart_putc( char c );
void uart_print(char *str);
void uart_print_hex32( unsigned int v );
void uart_print_hex64( uint64_t v );

#endif
