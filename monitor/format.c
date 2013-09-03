#include "format.h"
#include "arch_types.h"

#define PRINT_BUF_LEN   12

typedef enum {
    DECIMAL = 10,
    HEXADECIMAL = 16
} numerical_t;

static puts_t format_puts;
static putc_t format_putc;

static void format_printi( uint32_t v, numerical_t numerical, char base)
{
    char print_buf[PRINT_BUF_LEN];
    char *s;
    unsigned int mask8 = 0xF;
    unsigned int c;
    int i;
    switch(numerical){
    case DECIMAL:
        s = print_buf + PRINT_BUF_LEN - 1;
        *s = '\0';
        for(;v != 0UL;){
            *--s = ((v % 10) + '0');
            v /= 10;
        }
        format_puts(s);
        break;
    case HEXADECIMAL:
        for ( i = 7; i >= 0; i-- ) {
            c = (( v >> (i * 4) ) & mask8);
          if ( c < 10 ) {
                c += '0';
            } else {             
                c += base - 10;
            }
            format_putc( (char) c );
        }
        break;
    }
}

int format_print(const char *format, va_list ap)
{
    const char *p;
    for(p = format; *p != '\0'; p++){
        if(*p == '%'){
            ++p;
            if (*p == 'd'){
                format_printi(va_arg(ap, int), DECIMAL, 0);
            } 
            if (*p == 's'){
                format_puts(va_arg(ap, char*));
            }
            if (*p == 'c'){
                format_putc(va_arg(ap, int));
            }
            if (*p == 'x'){
                format_printi(va_arg(ap, unsigned int), HEXADECIMAL, 'a'); 
            }
            if (*p == 'X'){
                format_printi(va_arg(ap, unsigned int), HEXADECIMAL, 'A'); 
            }
        } else {
            if (*p == '\n'){
                format_putc(*p);
                format_putc('\r');
            }
            else if (*p == '\r'){
               
            }else{
                format_putc(*p);
            }
        }        
    }
    return 0;
}

void format_reg_puts(puts_t s)
{
    format_puts = s;
}

void format_reg_putc(putc_t c)
{
    format_putc = c;
}
