#include "semihosting.h"
#include "monitor_private.h"

void mon_enter_hyp(void)
{
	semi_write0("[monitor] enter_hyp: Not implemented\n");
	__mon_enter_hyp();
}
