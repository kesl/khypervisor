#include "format.h"
#include "arch_types.h"
#ifdef _SMP_
#include "smp.h"
#endif

#define PRINT_BUF_LEN   12

enum numerical {
    DECIMAL = 10,
    HEXADECIMAL = 16
};

static format_puts_t format_puts;
static format_putc_t format_putc;

#ifdef _SMP_
static DEFINE_SPINLOCK(format_lock);
#endif

static void format_printi(uint32_t v, enum numerical numerical, char base)
{
    char print_buf[PRINT_BUF_LEN];
    char *s;
    unsigned int mask8 = 0xF;
    unsigned int c;
    int i;
    switch (numerical) {
    case DECIMAL:
        s = print_buf + PRINT_BUF_LEN - 1;
        *s = '\0';
        if (v == 0UL)
            *--s = '0';
        else {
            for (; v != 0UL;) {
                *--s = ((v % 10) + '0');
                v /= 10;
            }
        }
        format_puts(s);
    break;
    case HEXADECIMAL:
        for (i = 7; i >= 0; i--) {
            c = ((v >> (i * 4)) & mask8);
            if (c < 10)
                c += '0';
            else
                c += base - 10;
            format_putc((char) c);
        }
    break;
    }
}

int format_print(const char *format, __builtin_va_list ap)
{
    const char *p;
    unsigned long flags;
#ifdef _SMP_
    smp_spin_lock(&format_lock, flags);
#endif
    for (p = format; *p != '\0'; p++) {
        if (*p == '%') {
            ++p;
            if (*p == 'd')
                format_printi(va_arg(ap, int), DECIMAL, 0);
            if (*p == 's')
                format_puts(va_arg(ap, char *));
            if (*p == 'c')
                format_putc(va_arg(ap, int));
            if (*p == 'x')
                format_printi(va_arg(ap, unsigned int), HEXADECIMAL, 'a');
            if (*p == 'X')
                format_printi(va_arg(ap, unsigned int), HEXADECIMAL, 'A');
        } else {
            if (*p == '\n') {
                format_putc(*p);
                format_putc('\r');
            } else
                format_putc(*p);
        }
    }
#ifdef _SMP_
    smp_spin_unlock(&format_lock, flags);
#endif

    return 0;
}

void format_reg_puts(format_puts_t s)
{
    format_puts = s;
}

void format_reg_putc(format_putc_t c)
{
    format_putc = c;
}
