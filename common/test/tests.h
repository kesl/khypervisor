#ifndef __TESTS__
#define __TESTS__

#include <hvmm_types.h>

/* Enable/Disable Test Items */

/*
 * GIC/Timer test disabled due to scheduler test does
 * context switching based on timer ticks
 */
#define TESTS_ENABLE_GIC_TIMER          0x01
#define TESTS_ENABLE_GIC_PWM_TIMER      0x02
#define TESTS_ENABLE_MALLOC             0x04
#define TESTS_ENABLE_VGIC               0x08
#define TESTS_VDEV                      0x10
#define TESTS_ENABLE_SP804              0x20

hvmm_status_t basic_tests_run(uint32_t tests);

#endif
