#ifndef __UART_PRINT_H__
#define __UART_PRINT_H__
#include "arch_types.h"

/** @brief Likes putc.
 *  @param v Character for print.
 */
void uart_putc(const char c);

/** @brief Likes puts. String only.
 *  @param v Strints for print.
 */
void uart_print(const char *str);

/** @brief Prints hex number(32bit).
 *  @param v Number for print.
 */
void uart_print_hex32(uint32_t v);

/** @brief Prints hex number(64bit).
 *  @param v Number for print.
 */
void uart_print_hex64(uint64_t v);

/** @brief Prints decimal number.
 *  @param v Number for print
 */
void uart_print_dec(uint32_t v);

/** @brief Likes getc. Reads character from the uart(stdin).
 *  @return Character from the uart's FIFO.
 */
char uart_getc(void);

/** @brief Likes gets. Reads characters from the uart(stdin).
 *         and stores them as a string into s
 *  @param s Return string from the uart.
 *  @param maxwidth Max length of read
 */
void uart_gets(char *s, int maxwidth);

/** @brief Initializes uart.
 *  @tobo Implemented about rtsm, not arndale yet.
 */
void uart_init(void);

/** @brief   Checks whether there is a data or not in the FIFO
 *  @return  There is a data in the FIFO then returns 1, otherwise returns 0.
 */
int uart_tst_fifo(void);

#endif
