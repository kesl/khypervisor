#include "uart_print.h"
/* UART Base Address determined by Hypervisor's Stage 2 Translation Table */
//#define UART0           0x3FCA0000
#define UART0           0x1C090000
static char _dummy_byte;


#ifndef __ASSEMBLY__
/* baudrate rest value */
union br_rest {
    unsigned short  slot;       /* udivslot */
    unsigned char   value;      /* ufracval */
};

struct s5p_uart {
    unsigned int    ulcon;
    unsigned int    ucon;
    unsigned int    ufcon;
    unsigned int    umcon;
    unsigned int    utrstat;
    unsigned int    uerstat;
    unsigned int    ufstat;
    unsigned int    umstat;
    unsigned char   utxh;
    unsigned char   res1[3];
    unsigned char   urxh;
    unsigned char   res2[3];
    unsigned int    ubrdiv;
    union br_rest   rest;
    unsigned char   res3[0xffd0];
};

static inline int s5p_uart_divslot(void)
{
    return 0;
}

#endif  /* __ASSEMBLY__ */



/* Exynos 5250 UART register macros */
#define UTXH        0x20
#define UFSTAT      0x18
#define UART_BASE   (UART0 + UTXH)

#define TX_FIFO_FULL_MASK       (1 << 24)
#define readl(a)         (*(volatile unsigned int *)(a))
#define writeb(v, a)         (*(volatile unsigned char *)(a) = (v))

static int serial_err_check(int op)
{
    struct s5p_uart *const uart = (struct s5p_uart *) UART0;
    unsigned int mask;

    if(op)
        mask = 0x8;
    else
        mask = 0xf;

    return readl(&uart->uerstat) & mask;

}

void uart_putc(const char c)
{
    struct s5p_uart *const uart = (struct s5p_uart *) UART0;


    while((readl(&uart->ufstat) & TX_FIFO_FULL_MASK)) {
        if (serial_err_check(1))
            return;
    }

    writeb(c, &uart->utxh);

    if(c == '\n')
        uart_putc('\r');


}
void uart_print(const char *str)
{
    while(*str) {
        uart_putc(*str++);
    }
}

void uart_print_hex32( uint32_t v )
{
    unsigned int mask8 = 0xF;
    unsigned int c;
    int i;
    uart_print("0x");

    for ( i = 7; i >= 0; i-- ) {
        c = (( v >> (i * 4) ) & mask8);
        if ( c < 10 ) {
            c += '0';
        } else {
            c += 'A' - 10;
        }
        uart_putc( (char) c );
    }
}

void uart_print_hex64( uint64_t v )
{
    uart_print_hex32( v >> 32 );
    uart_print_hex32( (uint32_t) (v & 0xFFFFFFFF) );
}

void uart_init(void)
{
    /* TODO:
       Figure out how to initialize the UART.
       Currently, intialized by Hypervisor for FastModels RTSM_VE, as a workaround
     */
    /* ibrd 0x24 */
    //UART_BASE[9] = 0x10;
    //UART_BASE[12] = 0xc300;
}
