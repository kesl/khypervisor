#ifndef __UART_PRINT_H__
#define __UART_PRINT_H__
#include "arch_types.h"


void uart_putc( const char c );
void uart_print(const char *str);
void uart_print_hex32( uint32_t v );
void uart_print_hex64( uint64_t v );

#endif
