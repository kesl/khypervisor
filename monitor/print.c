#include "print.h"
#include "format.h"
#include "uart_print.h"


void init_print()
{
    format_reg_putc(&uart_putc);
    format_reg_puts(&uart_print);
}
void printh(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    format_print(format, ap);
    va_end(ap);
}
void puts(const char* str)
{
    format_puts(str);
}
void putc(const char character)
{
    format_putc(character);
}

