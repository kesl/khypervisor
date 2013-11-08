#include "hyp_config.h"
#include "uart_print.h"
#include "mm.h"
#include "armv7_p15.h"
#include "arch_types.h"
#include <gic.h>
#include "interrupt.h"
#include "context.h"
#include "scheduler.h"
#include "tests.h"
#include "print.h"
#include <hvmm_trace.h>
#include <vdev.h>
#include "vdev/vdev_gicd.h"
#include "cfg_platform.h"
#include <gic_regs.h>

void hyp_main(void)
{
    init_print();

	hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;
    printh("[%s : %d] Starting...\n", __FUNCTION__, __LINE__);

	/* Initialize Memory Management */
	ret = hvmm_mm_init();

	/* Initialize Interrupt Management */
	ret = hvmm_interrupt_init();
	if ( ret != HVMM_STATUS_SUCCESS ) {
		uart_print("[hyp_main] interrupt initialization failed...\n\r");
	}

	/* Initialize Guests */
	context_init_guests();

	/* Initialize Virtual Devices */
	vdev_init();

    /* Virtual GIC Distributor */
    printh( "tests: Registering sample vdev:'vgicd' at %x\n", CFG_GIC_BASE_PA | GIC_OFFSET_GICD);
    vdev_gicd_init(CFG_GIC_BASE_PA | GIC_OFFSET_GICD);

    /* Start Scheduling */
	scheduler_test_scheduling();

    /* Begin running test code for newly implemented features*/
    hvmm_tests_main();

	/* Switch to the first guest */
	context_switch_to_initial_guest();

	/* The code flow must not reach here */
	uart_print("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n\r");
	hyp_abort_infinite();
}

