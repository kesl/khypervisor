#include <string.h>

static void *__memmove_down(void *__dest, __const void *__src, size_t __n)
{
    unsigned char *d = (unsigned char *)__dest, *s = (unsigned char *)__src;
    while (__n--)
        *d++ = *s++;

    return __dest;
}

static void *__memmove_up(void *__dest, __const void *__src, size_t __n)
{
    unsigned char *d = (unsigned char *)__dest + __n - 1, *s = \
                       (unsigned char *)__src + __n - 1;
    while (__n--)
        *d-- = *s--;

    return __dest;
}

void *(memcpy)(void *__dest, __const void *__src, size_t __n)
{
    return __memmove_down(__dest, __src, __n);
}

void *(memmove)(void *__dest, __const void *__src, size_t __n)
{
    if (__dest > __src)
        return __memmove_up(__dest, __src, __n);
    else
        return __memmove_down(__dest, __src, __n);
}

void *(memchr)(void const *s, int c, size_t n)
{
    unsigned char const *_s = (unsigned char const *)s;
    while (n && *_s != c) {
        ++_s;
        --n;
    }
    if (n)
        return (void *)_s;   /* the C library casts const away */
    else
        return (void *)0;
}

size_t (strlen)(const char *s)
{
    const char *sc = s;
    while (*sc != '\0')
        sc++;
    return sc - s;
}

void *(memset)(void *s, int c, size_t count)
{
    char *xs = s;
    while (count--)
        *xs++ = c;
    return s;
}

int (memcmp)(void const *p1, void const *p2, size_t n)
{
    unsigned char const *_p1 = p1;
    unsigned char const *_p2 = p2;
    while (n--) {
        if (*_p1 < *_p2)
            return -1;
        else if (*_p1 > *_p2)
            return 1;

        ++_p1;
        ++_p2;
    }
    return 0;
}

int (strcmp)(char const *s1, char const *s2)
{
    while (*s1 && *s2) {
        if (*s1 < *s2)
            return -1;
        else if (*s1 > *s2)
            return 1;

        ++s1;
        ++s2;
    }
    if (!*s1 && !*s2)
        return 0;
    else if (!*s1)
        return -1;
    else
        return 1;
}

int (strncmp)(char const *s1, char const *s2, size_t n)
{
    while (*s1 && *s2 && n--) {
        if (*s1 < *s2)
            return -1;
        else if (*s1 > *s2)
            return 1;
        ++s1;
        ++s2;
    }
    if (n == 0 || (!*s1 && !*s2))
        return 0;
    else if (!*s1)
        return -1;
    else
        return 1;
}

char *(strchr)(char const *s, int c)
{
    unsigned char const *_s = (unsigned char const *)s;
    while (*_s && *_s != c)
        ++_s;

    if (*_s)
        return (char *)_s;   /* the C library casts const away */
    else
        return (char *)0;
}

char *(strcpy)(char *dest, const char *src)
{
    unsigned int i;
    for (i = 0; src[i] != '\0'; ++i)
        dest[i] = src[i];

    dest[i] = '\0';
    return dest;
}

unsigned int arm_hexstr2uint(char *src)
{
    unsigned int val = 0x0;
    int pos = 0;

    if ((src[0] == '0') && (src[1] == 'x'))
        pos = 2;

    while (src[pos]) {
        if (('0' <= src[pos]) && (src[pos] <= '9'))
            val = val * 16 + (src[pos] - '0');
        else if (('A' <= src[pos]) && (src[pos] <= 'F'))
            val = val * 16 + (src[pos] - 'A' + 10);
        else if (('a' <= src[pos]) && (src[pos] <= 'f'))
            val = val * 16 + (src[pos] - 'a' + 10);
        pos++;
    }

    return val;
}

void arm_uint2hexstr(char *dst, unsigned int src)
{
    int ite, pos = 0;
    static const char hexchars[] = "0123456789ABCDEF";

    for (ite = 0; ite < 8; ite++) {
        if ((pos == 0) && !((src >> (4 * (8 - ite - 1))) &
                    0xF)) {
            continue;
        }
        dst[pos] = hexchars[(src >> (4 * (8 - ite -
                        1))) & 0xF];
        pos++;
    }
    if (pos == 0) {
        dst[pos] = '0';
        pos++;
    }
    dst[pos] = '\0';
}

char *strcat(char *dest, const char *src)
{
    while (*dest)
        dest++;

    while (*src) {
        *dest = *src;
        src++;
        dest++;
    }
    *dest = '\0';
    return dest;
}

int arm_str2int(char *src)
{
    int val = 0, pos = 0, minus = 0;

    if (src[pos] == '-') {
        minus = 1;
        pos++;
    }

    while (src[pos]) {
        val = 10 * val + (src[pos] - '0');
        pos++;
    }

    return (minus) ? -val : val;
}

