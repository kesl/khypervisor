#include "semihosting.h"

void _hyp_hvc_service(unsigned int hsr)
{
	unsigned int iss = hsr & 0xFFFF;
	semi_write0("[hyp] _hyp_hvc_service: enter\n");
	switch( iss ) {
		case 0xFFFE:
			// hyp_ping
			semi_write0("[hyp] _hyp_hvc_service:ping\n");
			break;
		default:
			break;
	}
	semi_write0("[hyp] _hyp_hvc_service: done\n");

}

void hyp_init_guests(void)
{
	semi_write0("[hyp] init_guests: enter\n");
	__mon_install_guest();
	semi_write0("[hyp] init_guests: return\n");
}

void hyp_switch_guest(void)
{
	semi_write0("[hyp] switch_guest: enter\n");
	__mon_switch_to_guest();
	semi_write0("[hyp] switch_guest: return\n");
}

void hyp_main(void)
{
	semi_write0("[hyp_main] Starting...\n");
	hyp_init_guests();

	hyp_switch_guest();

	semi_write0("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n");
}

