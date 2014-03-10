#include "pl01x.h"
#include "asm_io.h"

void pl01x_putc(const char c)
{
    /* Wait until there is space in the FIFO */
    while (*((uint32_t *)(PL01X_BASE + 0x18)) & 0x20)
        ;
    /* Send the character */
    volatile char *pUART = (char *) PL01X_BASE;
    *pUART = c;
}

char pl01x_getc(uint32_t base)
{
    char data;

    /* Wait until there is data in the FIFO */
    while (readl((void *)(base + UART_PL01x_FR)) & UART_PL01x_FR_RXFE)
        ;
    data = *((uint32_t *)(base + UART_PL01x_DR));

    /* Check for an error flag */
    if (data & 0xFFFFFF00) {
        /* Clear the error */
        volatile uint32_t *pUART = (uint32_t *) (PL01X_BASE + UART_PL01x_ECR);
        *pUART = 0xFFFFFFFF;
        return -1;
    }

    return data;
}

void pl01x_init(uint32_t base, uint32_t type, uint32_t baudrate,
        uint32_t input_clock)
{
    unsigned int divider;
    unsigned int temp;
    unsigned int remainder;
    unsigned int fraction;

    if (type == PL01X_TYPE_1) {
        /* First, disable everything */
        writel(0x0, (void *)(base + UART_PL011_CR));

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

        writel(divider, (void *)(base + UART_PL011_IBRD));
        writel(fraction, (void *)(base + UART_PL011_FBRD));

        /* Set the UART to be 8 bits, 1 stop bit,
         * no parity, fifo enabled
         */
        writel((UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN),
                (void *)(base + UART_PL011_LCRH));

        /* Finally, enable the UART */
        writel((UART_PL011_CR_UARTEN |
                UART_PL011_CR_TXE |
                UART_PL011_CR_RXE),
                (void *)(base + UART_PL011_CR));
    } else {
        /* First, disable everything */
        writel(0x0, (void *)(base + UART_PL010_CR));

        /* Set baud rate */
        switch (baudrate) {
        case 9600:
            divider = UART_PL010_BAUD_9600;
            break;
        case 19200:
            divider = UART_PL010_BAUD_9600;
            break;
        case 38400:
            divider = UART_PL010_BAUD_38400;
            break;
        case 57600:
            divider = UART_PL010_BAUD_57600;
            break;
        case 115200:
            divider = UART_PL010_BAUD_115200;
            break;
        default:
            divider = UART_PL010_BAUD_38400;
        }

        writel(((divider & 0xf00) >> 8),
                (void *)(base + UART_PL010_LCRM));
        writel((divider & 0xff), (void *)(base + UART_PL010_LCRL));

        /* Set the UART to be 8 bits, 1 stop bit,
         * no parity, fifo enabled */
        writel((UART_PL010_LCRH_WLEN_8 | UART_PL010_LCRH_FEN),
                (void *)(base + UART_PL010_LCRH));

        /* Finally, enable the UART */
        writel((UART_PL010_CR_UARTEN),
                (void *)(base + UART_PL010_CR));
    }
}
