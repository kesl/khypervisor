#include <arch_types.h>
#include <log/uart_print.h>

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
    uart_print("vdev_sample: Starting test..., base:");
    uart_print_hex32((uint32_t) base);
    uart_print("\n\r");
    for (i = 0; i < 10; i++) {
        v1 = (1 + i) * 2;
        v2 = (1 + i) * 3;
        uart_print("v1(");
        uart_print_hex32(v1);
        uart_print(")+v2(");
        uart_print_hex32(v2);
        uart_print(")\n\r");
        *reg_a = v1;
        *reg_b = v2;
        r = *reg_c;
        uart_print("    = r(");
        uart_print_hex32(r);
        if (r == (v1 + v2))
            uart_print(") - OK");
        else
            uart_print(") - FAILED");

        uart_print("\n\r");
    }
    uart_print("vdev_sample: End\n\r");
}
