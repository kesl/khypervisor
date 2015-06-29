#include "arch_types.h"
#include "uart_16550a.h"

volatile unsigned char *pUART;
/*
 * init sequence
 * - set ahbc setting 
 *    - setting ahbc clock & uart clock
 * - init uart
 */

struct _divisor {
    unsigned char dll;
    unsigned char dlm;
};

struct _divisor dvisor[] ={
    {0x01, 0x00}, // baud_115200
    {0x02, 0x00}, // baud_57600
    {0x03, 0x00}, // baud_38400
    {0x06, 0x00}, // baud_19200
    {0x0c, 0x00}, // baud_9600
};
#define UART_LCR_DEFAULT (UART_LCR_DWL_8|UART_LCR_SB_1|UART_LCR_NP)
void uart_16550a_init(unsigned long long base, enum baud_rate baud)
{
    pUART = (unsigned char *) base;

    /*
     * Setting Buad Rate (DLL, DLM)
     * 8N1 (8bits, No parity, 1 stop bit)
     */
    pUART[UART_IER] = 0x0;

    pUART[UART_LCR] = UART_LCR_DLAB_1 | UART_LCR_DEFAULT;
    pUART[UART_DLL] = 0;
    pUART[UART_DLM] = 0;
    pUART[UART_LCR] = UART_LCR_DEFAULT;
    pUART[UART_MCR] = UART_MCR_DTR | UART_MCR_RTS;
    pUART[UART_FCR] = (UART_FCR_ENABLE | UART_FCR_CLR_RX | UART_FCR_CLR_TX);
    pUART[UART_LCR] = UART_LCR_DLAB_1 | UART_LCR_DEFAULT;
    pUART[UART_DLL] = dvisor[baud].dll;
    pUART[UART_DLM] = dvisor[baud].dlm;
    pUART[UART_LCR] = UART_LCR_DEFAULT;
}

void uart_16550a_putc(const char c)
{
    int i;
    if (c == '\n')
        uart_16550a_putc('\r');
    for( i=1; i<3500; i++)
    {
        if ( (pUART[UART_LSR] & UART_LSR_THE) == 0x20)
            break;
        // have to? add deploy function
    }
    *pUART = c;
}

int uart_16550a_tst_fifo()
{
    /* There is not a data in the FIFO */
    if ((pUART[UART_LSR]&UART_LSR_DA) != 0x1)
        return 0;
    else
        /* There is a data in the FIFO */
        return 1;
}
char uart_16550a_getc()
{
    char data;
    while ((pUART[UART_LSR] & UART_LSR_DA) == 0x1)
        ;
    data = pUART[UART_RBR];
}

void uart_16550a_print(const char *str)
{
    while (*str)
        uart_16550a_putc(*str++);
}

void uart_16550a_print_hex(uint32_t v)
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
        uart_16550a_putc((char) c);
    }
}

void uart_16550a_print_hex32(uint32_t v)
{
    uart_16550a_print("0x");
    uart_16550a_print_hex(v);
}

void uart_16550a_print_hex64(uint64_t v)
{
    uart_print("0x");
    uart_16550a_print_hex(v >> 32);
    uart_16550a_print("_");
    uart_16550a_print_hex((uint32_t)(v & 0xFFFFFFFF));
}
