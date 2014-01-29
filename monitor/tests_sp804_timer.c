#include "hvmm_trace.h"
#include <gic.h>

/*
 * Configure sp804 timer irq for test
 */
hvmm_status_t hvmm_tests_sp804_timer(void)
{

    HVMM_TRACE_ENTER();

    /*
     * Until the guest enables IRQ34, for SP804, through VGICD, we manually enable here for 
     * test purpose so the hypervisor can receive and forward to a designated guest 
     */
    gic_test_configure_irq(34,
                           GIC_INT_POLARITY_LEVEL,
                           gic_cpumask_current(),
                           GIC_INT_PRIORITY_DEFAULT );

    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}

