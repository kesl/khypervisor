/* UART Base Address determined by Hypervisor's Stage 2 Translation Table */

#define UART_BASE       ((volatile unsigned int *) 0x1C090000)

static char _dummy_byte;

void uart_init(void)
{
    /* TODO:
       Figure out how to initialize the UART.
       Currently, intialized by Hypervisor for FastModels
       RTSM_VE, as a workaround
     */
    /* ibrd 0x24 */
    /* UART_BASE[9] = 0x10; */
    /* UART_BASE[12] = 0xc300; */
}

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
