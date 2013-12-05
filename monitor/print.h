#ifndef __PRINT_H__
#define __PRINT_H__

void init_print();

void printH(const char *format, ...);

#ifdef DEBUG
#define printh printH
#else
#define printh(format, args...) ((void)0)
#endif

#endif
