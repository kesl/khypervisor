#include "semihosting.h"

void hyp_init_guests(void)
{
	semi_write0("[hyp] init_guests: not implemented\n");
}

void hyp_switch_guest(void)
{
	semi_write0("[hyp] switch_guest: not implemented\n");
}

void hyp_main(void)
{
	semi_write0("[hyp_main] Starting...\n");
	hyp_init_guests();

	hyp_switch_guest();

	semi_write0("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n");
}

