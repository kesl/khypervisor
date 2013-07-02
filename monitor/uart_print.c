#include "arch_types.h"

#ifdef MACH_MPS
#define UART_BASE       0x1f005000
#elif defined (VEXPRESS)
#define UART_BASE       0x1c090000
#else
#define UART_BASE       0x10009000
#endif

void uart_print(char *str)
{
        volatile char *pUART = (char *) UART_BASE;
        while(*str) {
                *pUART = *str++;
        }
}

void uart_putc( char c )
{
        volatile char *pUART = (char *) UART_BASE;
	*pUART = c;
}

void uart_print_hex32( unsigned int v )
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
