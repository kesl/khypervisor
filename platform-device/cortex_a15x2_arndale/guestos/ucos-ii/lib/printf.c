#include "stdio.h"
#include "includes.h"

#define HIS_MAX               8

char HisBuff[HIS_MAX][32] = { { 0, }, };
int HisCount = 0;
int HisIndex = 0;

void uart_out_char_check_CR(const char c);
int uart_out_str_check_CR(char *str, int size);
void uart_tx_char(char c);

int putc(char c)
{
    uart_out_char_check_CR( c );
    return 1;
}

int putx(char c)
{
    uart_tx_char(c);
    return 1;
}

int printf(const char *fmt, ...)
{
    char buffer[1024];
    va_list ap;
    int len;

    va_start(ap, fmt);
    len = vsprintf(buffer, fmt, ap);
    va_end(ap);

    uart_out_str_check_CR(buffer, len);

    return len;
}

extern char uart_rx_char();
int getc(void)
{
    return uart_rx_char();
}

int gets(char *s)
{
    int cnt = 0;
    char c;

    while ((c = getc()) != CR) {
        if (c != BS) {
            cnt++;
            *s++ = c;
            printf("%c", c);
        } else {
            if (cnt > 0) {
                cnt--;
                *s-- = ' ';
                printf("\b \b");
            }

        }
    }
    *s = 0;

    return (cnt);
}

int serial_gets(char *buf, int size)
{
    int i;
    for (i = 0; i < size; i++)
        buf[i] = getc();

    return 0;
}
