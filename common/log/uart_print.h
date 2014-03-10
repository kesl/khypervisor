#ifndef __UART_PRINT_H__
#define __UART_PRINT_H__
#include "arch_types.h"

void uart_putc(const char c);
void uart_print(const char *str);
void uart_print_hex32(uint32_t v);
void uart_print_hex64(uint64_t v);
char uart_getc(void);
void uart_gets(char *s, int maxwidth, char endchar);
void uart_init(void);

#endif
