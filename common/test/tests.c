#include "tests.h"
#include "tests_gic_timer.h"
#include "tests_vdev.h"
#include "tests_malloc.h"

hvmm_status_t basic_tests_run(uint32_t tests)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    /* Entry point for sequence of test code */
    if (tests & TESTS_ENABLE_MALLOC)
        result = hvmm_tests_malloc();

    if (tests & TESTS_ENABLE_GIC_TIMER)
        result = hvmm_tests_gic_timer();

    if (tests & TESTS_ENABLE_VGIC)
        result = hvmm_tests_vgic();

    if (tests & TESTS_VDEV)
        result = hvmm_tests_vdev();

    return result;
}
