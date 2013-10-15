
#include "tests_vdev.h"
#include "vdev/vdev_sample.h"
#include "print.h"

hvmm_status_t hvmm_tests_vdev(void)
{
    printh( "tests: Registering sample vdev:'sample' at 0x3FFFF000\n");
    return vdev_sample_init(0x3FFFF000);
}

