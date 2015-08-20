#include "arch_types.h"
#include <log/uart_print.h>
#include "uart_16550a.h"
#include <log/print.h>

#define UART_BASE  UART_16550A_BASE
#define UART_BAUD  baud_115200
#define NULL    '\0'

void uart_putc(const char c)
{
    if (c == '\n')
        uart_16550a_putc('\r');
    uart_16550a_putc(c);
}

void uart_print(const char *str)
{
    while (*str) {
        uart_putc(*str);
        str++;
    }
}

#define UART_PRINT_BUF  12
void uart_print_dec(uint32_t v)
{
    char print_buf[UART_PRINT_BUF];
    char *s;
    int i;
    s = print_buf + UART_PRINT_BUF - 1;
    *s = NULL;
    if (v == 0UL)
        *--s = '0';
    else {
        for (; v != 0UL;) {
            *--s = ((v % 10) + '0');
            v /= 10;
        }
    }
    uart_print(s);
}

void uart_print_hex32(uint32_t v)
{
    uart_16550a_print_hex32(v);
}

void uart_print_hex64(uint64_t v)
{
    uart_16550a_print_hex64(v);
}

int uart_tst_fifo(void)
{
    if (!uart_16550a_tst_fifo())
        return 0;
    else
        return 1;
}

char uart_getc()
{
    char ch = uart_16550a_getc();
    if (ch == '\r')
        ch = '\n';
    uart_putc(ch);
    return ch;
}

void uart_gets(char *str, int max_column)
{
    char *retval;
    char ch;
    retval = str;
    ch = uart_getc();
    while (ch != '\n' && max_column > 0) {
        *retval = ch;
        retval++;
        max_column--;
        if (max_column == 0)
            break;
        ch = uart_getc();
    }
    *retval = NULL;
}

void uart_init(void)
{
    uart_16550a_init(UART_BASE, UART_BAUD);
    init_print();
}
