#include "arch_types.h"
#include <cfg_platform.h>


#ifdef CFG_GENERIC_CA15
#ifdef CFG_BOARD_RTSM_VE_CA15
#define UART0_BASE       0x1C090000
#else
#error "Configuration for board is not specified! GENERIC_CA15 but board is unknown."
#endif


void uart_print(const char *str)
{
        volatile char *pUART = (char *) UART0_BASE;
        while(*str) {
                *pUART = *str++;
        }
}

void uart_putc( const char c )
{
        volatile char *pUART = (char *) UART0_BASE;
	*pUART = c;
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
