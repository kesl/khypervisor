#include "stdio.h"
#include "uart_print.h"
#define va_start(v,l)       __builtin_va_start((v),l)
#define va_end              __builtin_va_end
#define va_arg              __builtin_va_arg
#define PRINT_BUF_LEN   12

typedef __builtin_va_list   va_list;
typedef enum {
    DECIMAL = 10,
    HEXADECIMAL = 16
} numerical_t;

void puts(const char* s)
{
    uart_print(s);
}

void putc(int character)
{
    uart_putc(character);
}

void printi( uint32_t v, numerical_t numerical, char base)
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
        uart_print(s);
        break;
    case HEXADECIMAL:
        for ( i = 7; i >= 0; i-- ) {
            c = (( v >> (i * 4) ) & mask8);
          if ( c < 10 ) {
                c += '0';
            } else {             
                c += base - 10;
            }
            uart_putc( (char) c );
        }
        break;
    }
}

int print(const char *format, va_list ap)
{
    const char *p;
    for(p = format; *p != '\0'; p++){
        if(*p == '%'){
            ++p;
            if (*p == 'd'){
                printi(va_arg(ap, int), DECIMAL, 0);
            } 
            if (*p == 's'){
                puts(va_arg(ap, char*));
            }
            if (*p == 'c'){
                putc(va_arg(ap, int));
            }
            if (*p == 'x'){
                printi(va_arg(ap, unsigned int), HEXADECIMAL, 'a'); 
            }
            if (*p == 'X'){
                printi(va_arg(ap, unsigned int), HEXADECIMAL, 'A'); 
            }
        } else {
            if (*p == '\n'){
                putc(*p);
                putc('\r');
            }
            else if (*p == '\r'){
               
            }else{
                putc(*p);
            }
        }        
    }
    return 0;
}

void printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    print(format, ap);
    va_end(ap);
}

