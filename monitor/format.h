#ifndef __FORMAT_H__
#define __FORMAT_H__
typedef void(*puts_t)(const char* str);
typedef void(*putc_t)(const char character);
puts_t puts;
putc_t putc;
void printh(const char *format, ...);
void reg_print(puts_t s, putc_t c);
#endif

