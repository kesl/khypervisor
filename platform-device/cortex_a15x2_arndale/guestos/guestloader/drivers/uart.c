#include "serial_s5p.h"
#include <log/uart_print.h>
#include <log/print.h>

void uart_putc(const char c)
{
    if (c == '\n')
        serial_putc('\r');
    serial_putc(c);
}

int uart_tst_fifo(void)
{
    if (!serial_tst_fifo())
        return 0;
    else
        return 1;
}

void uart_print(const char *str)
{
    while (*str)
        uart_putc(*str++);
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
        uart_putc((char)c);
    }
}

void uart_print_hex64(uint64_t v)
{
    uart_print_hex32(v >> 32);
    uart_print_hex32((uint32_t)(v & 0xFFFFFFFF));
}

#define NULL '\0'
char uart_getc(void)
{
    char ch = serial_getc();
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
        ch =
            uart_getc();
    }
    *retval = NULL;
}
void uart_init(void)
{
    serial_init();
    init_print();
}
