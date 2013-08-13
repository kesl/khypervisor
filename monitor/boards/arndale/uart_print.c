#include "arch_types.h"
#include "exynos-uart.h"
#include "cfg_platform.h"

#ifdef MACH_MPS
#define UART_BASE       0x1f005000
#elif defined (CFG_EXYNOS5250)
#define UART0       	0x12c20000
#define UTXH		0x20
#define UFSTAT		0x18
#define UART_BASE 	(UART0 + UTXH)
#else
#define UART_BASE       0x10009000
#endif

#if 0
void uart_print(const char *str)
{
        volatile char *pUART = (char *) UART_BASE;
        while(*str) {
                *pUART = *str++;
        }
}

void uart_putc( const char c )
{
        volatile char *pUART = (char *) UART_BASE;
	*pUART = c;
}
#endif

#define TX_FIFO_FULL_MASK       (1 << 24)
#define	readl(a)		 (*(volatile unsigned int *)(a))
#define	writeb(v, a)		 (*(volatile unsigned char *)(a) = (v))

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
