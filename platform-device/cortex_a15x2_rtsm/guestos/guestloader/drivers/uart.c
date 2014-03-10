#include "arch_types.h"
#include <log/uart_print.h>
#include "pl01x.h"
#define UART0_BASE  PL01X_BASE
void uart_putc(const char c)
{
    if (c == '\n')
        pl01x_putc('\r');
    pl01x_putc(c);
}

void uart_print(const char *str)
{
    while (*str) {
        uart_putc(*str);
        str++;
    }
    volatile char *pUART = (char *) UART0_BASE;
    while (*str)
        *pUART = *str++;
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

char uart_getc()
{
    char ch = pl01x_getc(UART0_BASE);
    if (ch == '\r')
        ch = '\n';
    uart_putc(ch);
    return ch;
}

void uart_gets(char *s, int maxwidth, char endchar)
{
    char *retval;
    char ch;
    retval = s;
    ch = uart_getc();
    while (ch != endchar && maxwidth > 0) {
        *retval = ch;
        retval++;
        maxwidth--;
        if (maxwidth == 0)
            break;
        ch = uart_getc();
    }
    *retval = '\0';
    return;
}

#define CA15X4_UART_BASE        0x1C090000
#define CA15X4_UART_TYPE        PL01X_TYPE_1
#define CA15X4_UART_INCLK       24000000
#define CA15X4_UART_BAUD        115200
void uart_init(void)
{
    pl01x_init(CA15X4_UART_BASE,
            CA15X4_UART_TYPE,
            CA15X4_UART_BAUD,
            CA15X4_UART_INCLK);
}
