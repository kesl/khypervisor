#include "pl011.h"
#include "asm_io.h"

void pl011_putc(const char c)
{
    /* Wait until there is space in the FIFO */
    while (*((uint32_t *)(PL011_BASE + 0x18)) & 0x20)
        ;
    /* Send the character */
    writel(c, (uint32_t *) PL011_BASE + PL011_UARTDR);
}

int pl011_tst_fifo(uint32_t base)
{
    /* There is not a data in the FIFO */
    if (readl((void *)(base + PL011_UARTFR)) & PL011_UARTFR_RXFE)
        return 0;
    else
        /* There is a data in the FIFO */
        return 1;
}

char pl011_getc(uint32_t base)
{
    char data;
    /* Wait until there is data in the FIFO */
    while (readl((void *)(base + PL011_UARTFR)) & PL011_UARTFR_RXFE)
        ;
    data = *((uint32_t *)(base + PL011_UARTDR));
    /* Check for an error flag */
    if (data & 0xFFFFFF00) {
        /* Clear the error */
        writel(0xFFFFFFFF, (uint32_t *) PL011_BASE + PL011_UARTECR);
        return -1;
    }
    return data;
}

void pl011_init(uint32_t base, uint32_t baudrate, uint32_t input_clock)
{
    unsigned int divider;
    unsigned int temp;
    unsigned int remainder;
    unsigned int fraction;

    /* First, disable everything */
    writel(0x0, (void *)(base + PL011_UARTCR));

    /*
     * Set baud rate
     *
     * IBRD = UART_CLK / (16 * BAUD_RATE)
     * FBRD = RND((64 * MOD(UART_CLK,(16 * BAUD_RATE)))
     *    / (16 * BAUD_RATE))
     */
    temp = 16 * baudrate;
    divider = input_clock / temp;
    remainder = input_clock % temp;
    temp = (8 * remainder) / baudrate;
    fraction = (temp >> 1) + (temp & 1);

    writel(divider, (void *)(base + PL011_UARTIBRD));
    writel(fraction, (void *)(base + PL011_UARTFBRD));

    /* Set the UART to be 8 bits, 1 stop bit,
     * no parity, fifo enabled
     */
    writel((PL011_UARTLCR_H_WLEN_8 | PL011_UARTLCR_H_FEN),
            (void *)(base + PL011_UARTLCR_H));

    /* Finally, enable the UART */
    writel((PL011_UARTCR_UARTEN | PL011_UARTCR_TXE | PL011_UARTCR_RXE),
            (void *)(base + PL011_UARTCR));
}
