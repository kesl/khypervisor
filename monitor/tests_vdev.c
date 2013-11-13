
#include "tests_vdev.h"
#include "vdev/vdev_sample.h"
#include "vdev/vdev_gicd.h"
#include "print.h"
#include <gic_regs.h>
#include <cfg_platform.h>

hvmm_status_t hvmm_tests_vdev(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;

    printh( "tests: Registering sample vdev:'sample' at 0x3FFFF000\n");
    result = vdev_sample_init(0x3FFFF000);

/*
    if ( result == HVMM_STATUS_SUCCESS ) {
        printh( "tests: Registering sample vdev:'vgicd' at %x\n", CFG_GIC_BASE_PA | GIC_OFFSET_GICD);
        result = vdev_gicd_init(CFG_GIC_BASE_PA | GIC_OFFSET_GICD);
    }
*/
    return result;
}

