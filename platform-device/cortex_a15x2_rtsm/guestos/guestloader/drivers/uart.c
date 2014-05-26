/* UART Base Address determined by Hypervisor's Stage 2 Translation Table */
#define UART_BASE       (0x1C090000)
#define UART_INCLK 24000000
#define UART_BAUD  38400
void uart_print(char *str)
{
    char *pUART = (char *) UART_BASE;
    while (*str)
        *pUART = *str++;
}
void uart_putc(char c)
{
    volatile char *pUART = (char *) UART_BASE;
    *pUART = c;
}
void uart_print_hex32(unsigned int v)
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

#define UART_CR 0x30
#define UART_IBRD 0x24
#define UART_FBRD 0x28

#define UART_LCR_H_WLEN_8 (3 << 5)
#define UART_LCR_H_FEN (1 << 4)
#define UART_LCR_H 0x2c

#define UART_CR_UARTEN (1 << 0)
#define UART_CR_TXE (1 << 8)
#define UART_CR_RXE (1 << 9)

void uart_init(void)
{
	unsigned int base;
	unsigned int baudrate;
	unsigned int input_clock;

    unsigned int divider;
    unsigned int temp;
    unsigned int remainder;
    unsigned int fraction;

    base = UART_BASE;
    baudrate = UART_BAUD;
    input_clock = UART_INCLK;

    /* First, disable everything */
	(*(volatile unsigned int *)(base + UART_CR)) = 0x0;

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

	(*(volatile unsigned int *)(base + UART_IBRD)) = divider;
	(*(volatile unsigned char *)(base + UART_FBRD)) = fraction;

    /* Set the UART to be 8 bits, 1 stop bit,
     * no parity, fifo enabled
     */
	(*(volatile unsigned char*)(base + UART_LCR_H))
		= (UART_LCR_H_WLEN_8 | UART_LCR_H_FEN);

    /* Finally, enable the UART */
	(*(volatile unsigned int*)(base + UART_CR))
		= (UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE);
}
