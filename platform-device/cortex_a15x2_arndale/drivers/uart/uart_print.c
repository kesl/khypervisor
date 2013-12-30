#include "arch_types.h"
#include "exynos-uart.h"

#include <config/cfg_platform.h>

#ifdef CFG_EXYNOS5250
#ifdef CFG_BOARD_ARNDALE
#define UART2_BASE       	0x12c20000
#else
#error "Configuration for board is not specified! Exynos5250 but board is unknow."
#endif

/* Exynos 5250 UART register macros */
#define UTXH		0x20
#define UFSTAT		0x18
#define UART_BASE 	(UART0 + UTXH)

#define TX_FIFO_FULL_MASK       (1 << 24)
#define	readl(a)		 (*(volatile unsigned int *)(a))
#define	writeb(v, a)		 (*(volatile unsigned char *)(a) = (v))

static int serial_err_check(int op)
{
	struct s5p_uart *const uart = (struct s5p_uart *) UART2_BASE;
	unsigned int mask;

	if(op)
		mask = 0x8;
	else
		mask = 0xf;

	return readl(&uart->uerstat) & mask;

}

void uart_putc(const char c)
{
	struct s5p_uart *const uart = (struct s5p_uart *) UART2_BASE;


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
#endif
