#include "arch_types.h"
#include <log/uart_print.h>
#include "pl011.h"

#define UART_BASE  PL011_BASE
#define UART_INCLK 24000000
#define UART_BAUD  115200
#define NULL    '\0'

void uart_putc(const char c)
{
    if (c == '\n')
        pl011_putc('\r');
    pl011_putc(c);
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
    unsigned int mask8 = 0xF;
    unsigned int c;
    int i;
    uart_print("0x");
    for (i = 7; i >= 0; i--) {
        c = ((v >> (i * 4)) & mask8);
        if (c < 10)
            c += '0';
        else
            c += 'A' - 10;
        uart_putc((char) c);
    }
}

void uart_print_hex64(uint64_t v)
{
    uart_print_hex32(v >> 32);
    uart_print_hex32((uint32_t)(v & 0xFFFFFFFF));
}

int uart_tst_fifo(void)
{
    if (!pl011_tst_fifo(UART_BASE))
        return 0;
    else
        return 1;
}

char uart_getc()
{
    char ch = pl011_getc(UART_BASE);
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
    pl011_init(UART_BASE, UART_BAUD, UART_INCLK);
}
