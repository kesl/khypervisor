/* UART Base Address determined by Hypervisor's Stage 2 Translation Table */

#define UART_BASE       (0x1C020000)

static char _dummy_byte;

volatile unsigned char *pUART;
#define UART_LCR 0x03
#define UART_FCR 0x02
#define UART_IER 0x01
#define UART_LSR 0x05

void uart_init(void)
{
    /* TODO:
       Figure out how to initialize the UART.
       Currently, intialized by Hypervisor for FastModels
       RTSM_VE, as a workaround
     */
    pUART = (unsigned char *) UART_BASE;

    // no ahbc setting, no clock setting
//    pUART[UART_LCR] = 0x03;
//    pUART[UART_FCR] = 0x00;
//    pUART[UART_IER] = 0x00;
}

void uart_putc(char c)
{
    int i;
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

void uart_print(char *str)
{
    while (*str)
        uart_putc(*str++);
}

void uart_print_hex(unsigned int v)
{
    unsigned int mask8 = 0xF;
    unsigned int c;
    int i;
    for (i = 7; i >= 0; i--) {
        c = ((v >> (i * 4)) & mask8);
        if (c < 10)
            c += '0';
        else
            c += 'A' - 10;
        uart_putc((char) c);
    }
}

void uart_print_hex32(unsigned int v)
{
    uart_print("0x");
    uart_print_hex(v);
}

void uart_print_hex64(unsigned long long v)
{
    uart_print("0x");
    uart_print_hex(v >> 32);
    uart_print("_");
    uart_print_hex((unsigned int)(v & 0xFFFFFFFF));
}
