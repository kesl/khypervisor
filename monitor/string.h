#ifndef STRING_H
#define STRING_H

#include <stddef.h>

extern void *(memcpy)(void *__dest, __const void *__src, size_t __n);
extern void *(memmove)(void *__dest, __const void *__src, size_t __n);
extern void *(memchr)(void const *s, int c, size_t n);
extern size_t (strlen)(const char *s);
extern void *(memset)(void *s, int c, size_t count);
extern int (memcmp)(void const *p1, void const *p2, size_t n);
extern int (strcmp)(char const *s1, char const *s2);
extern int (strncmp)(char const *s1, char const *s2, size_t n);
extern char *(strchr)(char const *s, int c);

#endif
