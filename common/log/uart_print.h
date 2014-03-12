#ifndef __UART_PRINT_H__
#define __UART_PRINT_H__
#include "arch_types.h"

void uart_putc(const char c);
/**
 * @brief   Likes puts. string only.
 */
void uart_print(const char *str);
void uart_print_hex32(uint32_t v);
void uart_print_hex64(uint64_t v);
/**
 * @brief   prints decimal.
 */
void uart_print_dec(uint32_t v);
char uart_getc(void);
void uart_gets(char *s, int maxwidth, char endchar);
/**
 * @tobo Implemented about rtsm, not ardale yet.
 */
void uart_init(void);
/**
 * @brief   Checks whether there is a data or not in the FIFO
 * @return  There is a data in the FIFO then returns 1, otherwise returns 0.
 */
int uart_tst_fifo(void);

#endif
