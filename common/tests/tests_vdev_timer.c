#include "vdev/vdev_timer.h"

hvmm_status_t hvmm_tests_vdev_timer(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    /* Injection by Generic TIMER Enabler */
    result = vdev_timer_init(0x3FFFE000);
    return result;
}

