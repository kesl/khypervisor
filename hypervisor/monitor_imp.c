#include "monitor_private.h"

#include <log/uart_print.h>

void mon_enter_hyp(void)
{
	uart_print("[monitor] enter_hyp: entering hyp mode. not coming back\n\r");
	__mon_enter_hyp();
}
