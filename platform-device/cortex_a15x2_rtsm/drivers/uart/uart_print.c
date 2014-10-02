#include "arch_types.h"
#include <k-hypervisor-config.h>


#ifdef CFG_GENERIC_CA15
#ifdef CFG_BOARD_RTSM_VE_CA15
#define UART0_BASE       0x1C090000
#define UART0_FR    0x18
#define UART0_FR_TXFF   0x20
#else
#error "Configuration for board is not specified!"\
   " GENERIC_CA15 but board is unknown."
#endif


void uart_print(const char *str)
{
    while (*str)
        uart_putc(*str++);
}

void uart_putc(const char c)
{
    /* Wait until there is space in the FIFO */
    while (*((uint32_t *)(UART0_BASE + UART0_FR)) & UART0_FR_TXFF)
        ;
    /* Send the character */
    volatile char *pUART = (char *) UART0_BASE;
    *pUART = c;
}

void uart_print_hex32(uint32_t v)
{
    unsigned int mask8 = 0xF;
    unsigned int c;
    int i;
    uart_print("0x");
    for (i = 7; i >= 0; i--) {
        c = ((v >> (i * 4)) & mask8);
        if (c < 10)
            c += '0';
        else
            c += 'A' - 10;
        uart_putc((char) c);
    }
}

void uart_print_hex64(uint64_t v)
{
    uart_print_hex32(v >> 32);
    uart_print_hex32((uint32_t)(v & 0xFFFFFFFF));
}

#endif
