#include "serial_s5p.h"
#include <include/asm_io.h>

#define TX_FIFO_FULL_MASK       (1 << 24)
#define RX_FIFO_COUNT_MASK  0xff
#define RX_FIFO_FULL_MASK   (1 << 8)

static int serial_err_check(int op)
{
    struct s5p_uart *const uart = (struct s5p_uart *) UART_BASE;
    unsigned int mask;
    if (op)
        mask = 0x8;
    else
        mask = 0xf;
    return readl(&uart->uerstat) & mask;
}

int serial_tst_fifo(void)
{
    struct s5p_uart *const uart = (struct s5p_uart *) UART_BASE;

    /* There is not a data in the FIFO */
    while (!(readl(&uart->ufstat) & (RX_FIFO_COUNT_MASK |
                    RX_FIFO_FULL_MASK))) {
            return 0;
    }
   /* There is a data in the FIFO */
    return 1;
}

int serial_getc(void)
{
    struct s5p_uart *const uart = (struct s5p_uart *) UART_BASE;

    /* wait for character to arrive */
    while (!(readl(&uart->ufstat) & (RX_FIFO_COUNT_MASK |
                    RX_FIFO_FULL_MASK))) {
        if (serial_err_check(0))
            return 0;
    }
    return (int)(readb(&uart->urxh) & 0xff);
}

void serial_putc(const char c)
{
    struct s5p_uart *const uart = (struct s5p_uart *) UART_BASE;
    while ((readl(&uart->ufstat) & TX_FIFO_FULL_MASK)) {
        if (serial_err_check(1))
            return;
    }
    writeb(c, &uart->utxh);
}
#define UART_BAUD  115200
#define CONFIG_SYS_CLK_FREQ     24000000
void serial_setbrg_dev(uint32_t base)
{
    struct s5p_uart *const uart = (struct s5p_uart *)base;
    uint32_t uclk = CONFIG_SYS_CLK_FREQ;
    uint32_t baudrate = UART_BAUD;
    uint32_t val;

    val = uclk / baudrate;

    writel(val / 16 - 1, &uart->ubrdiv);

    writeb(val % 16,
            &uart->rest.value);

}
void serial_init(void)
{
    struct s5p_uart *const uart = (struct s5p_uart *)UART_BASE;
    /* enable FIFOs */
    writel(0x1, &uart->ufcon);
    writel(0, &uart->umcon);
    /* 8N1 */
    writel(0x3, &uart->ulcon);
    /* No interrupts, no DMA, pure polling */
    writel(0x245, &uart->ucon);
}
