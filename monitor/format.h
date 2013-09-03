#ifndef __FORMAT_H__
#define __FORMAT_H__

#define va_start(v,l)       __builtin_va_start((v),l)
#define va_end              __builtin_va_end
#define va_arg              __builtin_va_arg
typedef __builtin_va_list   va_list;

typedef void(*puts_t)(const char* str);
typedef void(*putc_t)(const char character);

int format_print(const char *format, va_list ap);
void format_reg_puts(puts_t s);
void format_reg_putc(putc_t c);
#endif

