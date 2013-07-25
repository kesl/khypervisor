
#include "tests.h"
#include "tests_gic_timer.h"

/* Enable/Disable Test Items */

/* GIC/Timer test disabled due to scheduler test does context switching based on timer ticks */
/* #define TESTS_ENABLE_GIC_TIMER */

#define TESTS_ENABLE_VGIC

hvmm_status_t hvmm_tests_main(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;

    /* Entry point for sequence of test code */

#ifdef TESTS_ENABLE_GIC_TIMER
    result = hvmm_tests_gic_timer();
#endif

#ifdef TESTS_ENABLE_VGIC
    result = hvmm_tests_vgic();
#endif

    return result;
}
