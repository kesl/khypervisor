#include <io_emul.h>
#include <arch_types.h>
#include "uart_print.h"

#define REG_A	0x3FFFF000
#define REG_B	0x3FFFF004
#define REG_C	0x3FFFF008



#if 1
void readA()
{
	uart_print_hex32(readl(REG_A));
}
void readB()
{
	uart_print_hex32(readl(REG_B));
}
void readC()
{
	uart_print_hex32(readl(REG_C));
}

void writeA()
{
	writel(REG_A, 0x12340000);
}

void writeB()
{
	writel(REG_B, 0xFFFF);
}

void writeC()
{
	writel(REG_C, 0xFFFF1234);
}

#endif
