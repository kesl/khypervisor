#include "includes.h"

/* UART Base Address determined by Hypervisor's Stage 2 Translation Table */
#define UART0           0x12C20000

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

/* Exynos 5250 UART register macros */
#define UTXH        0x20
#define UFSTAT      0x18
#define UART_BASE   (UART0 + UTXH)

#define RX_FIFO_COUNT_MASK  0xff
#define RX_FIFO_FULL_MASK   (1 << 8)
#define TX_FIFO_FULL_MASK   (1 << 24)

#define readl(a)         (*(volatile unsigned int *)(a))
#define readb(a)         (*(volatile unsigned char *)(a))
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

static inline int s5p_uart_divslot(void)
{
    return 0;
}


void uart_tx_char(char c)
{
    struct s5p_uart *const uart = (struct s5p_uart *) UART0;


    while((readl(&uart->ufstat) & TX_FIFO_FULL_MASK)) {
        if (serial_err_check(1))
            return;
    }

    writeb(c, &uart->utxh);
}

void uart_out_char_check_CR(const char c)
{
    uart_tx_char(c);
    if (c == '\n')
        uart_tx_char('\r');
}

int uart_out_str(char *str, int size)
{
    int lp;

    for (lp = 0; lp < size; lp++)
        uart_tx_char(str[lp]);

    return lp;
}

int uart_out_str_check_CR(char *str, int size)
{
    int lp;

    for (lp = 0; lp < size; lp++)
        uart_out_char_check_CR(str[lp]);

    return lp;
}


int serial_getc_dev(void)
{
    struct s5p_uart * const uart = (struct s5p_uart *) UART0;

    /* wait for character to arrive */
    while (!(readl(&uart->ufstat) & (RX_FIFO_COUNT_MASK | RX_FIFO_FULL_MASK))) {
        if (serial_err_check(0))
            return 0;
    }

    return (int) (readb(&uart->urxh) & 0xff);
}

char uart_rx_char()
{
    return 0;
}

void uart_init()
{
    return;
}


