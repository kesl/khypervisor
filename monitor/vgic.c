#include "vgic.h"
#include "gic_regs.h"
#include "hvmm_trace.h"

/*
 * Context Switch:
 *  Saved/Restored Registers:
 *      - GICH_LR
 *      - GICH_APR
 *      - GICH_HCR
 *  Saved/Restored Data:
 *      - Free Interrupts
 *
 */
hvmm_status_t vgic_init(void)
{

    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    HVMM_TRACE_ENTER();

    /* Number of List Registers */


    HVMM_TRACE_EXIT();
    return result;
}

