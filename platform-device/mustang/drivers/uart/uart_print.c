#include "arch_types.h"
#include <k-hypervisor-config.h>
#include <uart_16650a.h>


#ifdef CFG_GENERIC_CA15
#ifdef CFG_BOARD_RTSM_VE_CA15
#define UART0_BASE       0x1C020000
#else
#error "Configuration for board is not specified!"\
   " GENERIC_CA15 but board is unknown."
#endif
volatile unsigned char *pUART;
/*
 * init sequence
 * - set ahbc setting 
 *    - setting ahbc clock & uart clock
 * - init uart
 */
void uart_init(void)
{
    pUART = (unsigned char *) UART0_BASE;

    // no ahbc setting, no clock setting
    pUART[UART_LCR] = 0x03;
    pUART[UART_FCR] = 0x00;
    pUART[UART_IER] = 0x00;

}


void uart_print(const char *str)
{
    while (*str)
        uart_putc(*str++);
}

void uart_putc(const char c)
{
    int i, j;
    if (c == '\n')
        uart_putc('\r');
    for( i=1; i<3500; i++)
    {
        if ( (pUART[UART_LSR] & 0x20) == 0x20)
            break;
        // have to? add deploy function
    }
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
