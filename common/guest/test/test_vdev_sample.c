#include <arch_types.h>
#include <log/uart_print.h>
#include <log/print.h>

#define VDEV_SAMPLE_BASE    0x3FFFF000
#define VDEV_OFFSET_REGA    0x00
#define VDEV_OFFSET_REGB    (0x04 / 4)
#define VDEV_OFFSET_REGC    (0x08 / 4)
#define VDEV_OFFSET_REGD    (0x0C / 4)

void test_vdev_sample()
{
    volatile uint32_t *base = (uint32_t *) VDEV_SAMPLE_BASE;
    volatile uint32_t *reg_a = base + VDEV_OFFSET_REGA;
    volatile uint32_t *reg_b = base + VDEV_OFFSET_REGB;
    volatile uint32_t *reg_c = base + VDEV_OFFSET_REGC;
    int i;
    int v1, v2, r;
    printH("vdev_sample: Starting test..., base:%x\n\r",(uint32_t) base);
    for (i = 0; i < 10; i++) {
        v1 = (1 + i) * 2;
        v2 = (1 + i) * 3;
        printH("v1(%x)+v2(%x)\n\r",v1,v2);
        *reg_a = v1;
        *reg_b = v2;
        r = *reg_c;
        if (r == (v1 + v2))
            printH("    = r(%x) - OK",r);
        else
            printH("    = r(%x) - FAILED",r);

        printH("\n\r");
    }
    printH("vdev_sample: End\n\r");
}
