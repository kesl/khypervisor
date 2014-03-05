
#include "tests_vdev.h"
#include "vdev/vdev_sample.h"
#include "vdev/vdev_gicd.h"
#include <gic_regs.h>
#include <asm-arm_inline.h>
#include <gic.h>
#include "virqmap.h"
#include <hyp_config.h>

#include <config/cfg_platform.h>
#include <log/print.h>

/* return the bit position of the first bit set from msb
 * for example, firstbit32(0x7F = 111 1111) returns 7
 */
#define firstbit32(word) (31 - asm_clz(word))
#define NUM_MAX_VIRQS   128
#define NUM_STATUS_WORDS    (NUM_MAX_VIRQS / 32)

/* old status */
static uint32_t ostatus[NUM_GUESTS_STATIC][NUM_STATUS_WORDS] = {{0, },};

void _my_vgicd_changed_istatus(vmid_t vmid, uint32_t istatus,
        uint8_t word_offset)
{
    uint32_t cstatus;                          /* changed bits only */
    uint32_t minirq;
    int bit;

    /* irq range: 0~31 + word_offset * size_of_istatus_in_bits */
    minirq = word_offset * 32;
    cstatus = ostatus[vmid][word_offset] ^ istatus;   /* find changed bits */
    while (cstatus) {
        uint32_t virq;
        uint32_t pirq;
        bit = firstbit32(cstatus);
        virq = minirq + bit;
        pirq = virqmap_pirq(vmid, virq);
        if (pirq != PIRQ_INVALID) {
            /* changed bit */
            if (istatus & (1 << bit)) {
                printh("[%s : %d] enabled irq num is %d\n",
                        __func__, __LINE__, bit + minirq);
                gic_test_configure_irq(pirq, GIC_INT_POLARITY_LEVEL,
                        gic_cpumask_current(), GIC_INT_PRIORITY_DEFAULT);
            } else {
                printh("[%s : %d] disabled irq num is %d\n",
                        __func__, __LINE__, bit + minirq);
                gic_disable_irq(pirq);
            }
        } else {
            printh("WARNING: Ignoring virq %d for guest"
                   "%d has no mapped pirq\n", virq, vmid);
        }
        cstatus &= ~(1 << bit);
    }
    ostatus[vmid][word_offset] = istatus;
}


hvmm_status_t hvmm_tests_vdev(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    printh("tests: Registering sample vdev:'sample' at 0x3FFFF000\n");
    result = vdev_sample_init(0x3FFFF000);
    return result;
}

hvmm_status_t hvmm_tests_vdev_gicd(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    printh("[%s : %d] register vgicd_changed_istatus callback.\n",
            __func__, __LINE__);
    vgicd_set_callback_changed_istatus(&_my_vgicd_changed_istatus);
    return result;
}

