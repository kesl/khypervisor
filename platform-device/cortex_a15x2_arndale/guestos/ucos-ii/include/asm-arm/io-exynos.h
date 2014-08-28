#ifndef __IO_EXYNOS_H__
#define __IO_EXYNOS_H_
/*
 * For write data on physical address.
 */
#include "arch_types.h"
#include "asm-arm_inline.h"
#define __raw_write32(a, v) (*(volatile uint32_t *)(a) = (v))
#define __raw_read32(a)     (*(volatile uint32_t *)(a))
#define __iowmb()   dsb()
#define __iormb()   dsb()
#define arch_out_le32(a, v) {__iowmb(); __raw_write32(a, v); }
#define arch_in_le32(a)     ({uint32_t v = __raw_read32(a); __iormb(); v; })



#define __arch_getb(a)          (*(volatile unsigned char *)(a))
#define __arch_getw(a)          (*(volatile unsigned short *)(a))
#define __arch_getl(a)          (*(volatile unsigned int *)(a))

#define __arch_putb(v,a)        (*(volatile unsigned char *)(a) = (v))
#define __arch_putw(v,a)        (*(volatile unsigned short *)(a) = (v))
#define __arch_putl(v,a)        (*(volatile unsigned int *)(a) = (v))


#define writeb(v,c) ({ uint8_t  __v = v; __iowmb(); __arch_putb(__v,c); __v; })
#define writew(v,c) ({ uint16_t __v = v; __iowmb(); __arch_putw(__v,c); __v; })
#define writel(v,c) ({ uint32_t __v = v; __iowmb(); __arch_putl(__v,c); __v; })

#define readb(c)    ({ uint8_t  __v = __arch_getb(c); __iormb(); __v; })
#define readw(c)    ({ uint16_t __v = __arch_getw(c); __iormb(); __v; })
#define readl(c)    ({ uint32_t __v = __arch_getl(c); __iormb(); __v; })


static inline uint32_t vmm_readl(volatile void *addr)
{
    return arch_in_le32(addr);
}
static inline void vmm_writel(uint32_t data, volatile void *addr)
{
    arch_out_le32(addr, data);
}


#define DEVICE_NOT_AVAILABLE        0

#define EXYNOS_CPU_NAME         "Exynos"
#define EXYNOS4_ADDR_BASE       0x10000000

/* EXYNOS4 */
#define EXYNOS4_GPIO_PART3_BASE     0x03860000
#define EXYNOS4_PRO_ID          0x10000000
#define EXYNOS4_SYSREG_BASE     0x10010000
#define EXYNOS4_POWER_BASE      0x10020000
#define EXYNOS4_SWRESET         0x10020400
#define EXYNOS4_CLOCK_BASE      0x10030000
#define EXYNOS4_SYSTIMER_BASE       0x10050000
#define EXYNOS4_WATCHDOG_BASE       0x10060000
#define EXYNOS4_MIU_BASE        0x10600000
#define EXYNOS4_DMC0_BASE       0x10400000
#define EXYNOS4_DMC1_BASE       0x10410000
#define EXYNOS4_GPIO_PART2_BASE     0x11000000
#define EXYNOS4_GPIO_PART1_BASE     0x11400000
#define EXYNOS4_FIMD_BASE       0x11C00000
#define EXYNOS4_MIPI_DSIM_BASE      0x11C80000
#define EXYNOS4_USBOTG_BASE     0x12480000
#define EXYNOS4_MMC_BASE        0x12510000
#define EXYNOS4_SROMC_BASE      0x12570000
#define EXYNOS4_USB_HOST_EHCI_BASE  0x12580000
#define EXYNOS4_USBPHY_BASE     0x125B0000
#define EXYNOS4_UART_BASE       0x13800000
#define EXYNOS4_I2C_BASE        0x13860000
#define EXYNOS4_ADC_BASE        0x13910000
#define EXYNOS4_PWMTIMER_BASE       0x139D0000
#define EXYNOS4_MODEM_BASE      0x13A00000
#define EXYNOS4_USBPHY_CONTROL      0x10020704

#define EXYNOS4_GPIO_PART4_BASE     DEVICE_NOT_AVAILABLE
#define EXYNOS4_DP_BASE         DEVICE_NOT_AVAILABLE

/* EXYNOS5 */
#define EXYNOS5_I2C_SPACING     0x10000

#define EXYNOS5_GPIO_PART4_BASE     0x03860000
#define EXYNOS5_PRO_ID          0x10000000
#define EXYNOS5_CLOCK_BASE      0x10010000
#define EXYNOS5_POWER_BASE      0x10040000
#define EXYNOS5_SWRESET         0x10040400
#define EXYNOS5_SYSREG_BASE     0x10050000
#define EXYNOS5_WATCHDOG_BASE       0x101D0000
#define EXYNOS5_GIC_DIST_BASE       0x10481000
#define EXYNOS5_GIC_CPU_BASE        0x10482000

#define EXYNOS5_DMC_PHY0_BASE       0x10C00000
#define EXYNOS5_DMC_PHY1_BASE       0x10C10000
#define EXYNOS5_GPIO_PART3_BASE     0x10D10000
#define EXYNOS5_DMC_CTRL_BASE       0x10DD0000
#define EXYNOS5_GPIO_PART1_BASE     0x11400000
#define EXYNOS5_MIPI_DSIM_BASE      0x11D00000
#define EXYNOS5_USB_HOST_EHCI_BASE  0x12110000
#define EXYNOS5_USBPHY_BASE     0x12130000
#define EXYNOS5_USBOTG_BASE     0x12140000
#define EXYNOS5_SATA_PHY_BASE           0x12170000
#define EXYNOS5_SATA_PHY_I2C            0x121D0000
#define EXYNOS5_MMC_BASE        0x12200000
#define EXYNOS5_SROMC_BASE      0x12250000
#define EXYNOS5_SATA_BASE               0x122F0000
#define EXYNOS5_UART_BASE       0x12C00000
#define EXYNOS5_I2C_BASE        0x12C60000
#define EXYNOS5_PWMTIMER_BASE       0x12DD0000
#define EXYNOS5_GPIO_PART2_BASE     0x13400000
#define EXYNOS5_FIMD_BASE       0x14400000
#define EXYNOS5_DP_BASE         0x145B0000

#define EXYNOS5_ADC_BASE        DEVICE_NOT_AVAILABLE
#define EXYNOS5_MODEM_BASE      DEVICE_NOT_AVAILABLE

struct s5p_gpio_bank {
    unsigned int    con;
    unsigned int    dat;
    unsigned int    pull;
    unsigned int    drv;
    unsigned int    pdn_con;
    unsigned int    pdn_pull;
    unsigned char   res1[8];
};

struct exynos4_gpio_part1 {
    struct s5p_gpio_bank a0;
    struct s5p_gpio_bank a1;
    struct s5p_gpio_bank b;
    struct s5p_gpio_bank c0;
    struct s5p_gpio_bank c1;
    struct s5p_gpio_bank d0;
    struct s5p_gpio_bank d1;
    struct s5p_gpio_bank e0;
    struct s5p_gpio_bank e1;
    struct s5p_gpio_bank e2;
    struct s5p_gpio_bank e3;
    struct s5p_gpio_bank e4;
    struct s5p_gpio_bank f0;
    struct s5p_gpio_bank f1;
    struct s5p_gpio_bank f2;
    struct s5p_gpio_bank f3;
};

struct exynos4_gpio_part2 {
    struct s5p_gpio_bank j0;
    struct s5p_gpio_bank j1;
    struct s5p_gpio_bank k0;
    struct s5p_gpio_bank k1;
    struct s5p_gpio_bank k2;
    struct s5p_gpio_bank k3;
    struct s5p_gpio_bank l0;
    struct s5p_gpio_bank l1;
    struct s5p_gpio_bank l2;
    struct s5p_gpio_bank y0;
    struct s5p_gpio_bank y1;
    struct s5p_gpio_bank y2;
    struct s5p_gpio_bank y3;
    struct s5p_gpio_bank y4;
    struct s5p_gpio_bank y5;
    struct s5p_gpio_bank y6;
    struct s5p_gpio_bank res1[80];
    struct s5p_gpio_bank x0;
    struct s5p_gpio_bank x1;
    struct s5p_gpio_bank x2;
    struct s5p_gpio_bank x3;
};

struct exynos4_gpio_part3 {
    struct s5p_gpio_bank z;
};

struct exynos5_gpio_part1 {
    struct s5p_gpio_bank a0;
    struct s5p_gpio_bank a1;
    struct s5p_gpio_bank a2;
    struct s5p_gpio_bank b0;
    struct s5p_gpio_bank b1;
    struct s5p_gpio_bank b2;
    struct s5p_gpio_bank b3;
    struct s5p_gpio_bank c0;
    struct s5p_gpio_bank c1;
    struct s5p_gpio_bank c2;
    struct s5p_gpio_bank c3;
    struct s5p_gpio_bank d0;
    struct s5p_gpio_bank d1;
    struct s5p_gpio_bank y0;
    struct s5p_gpio_bank y1;
    struct s5p_gpio_bank y2;
    struct s5p_gpio_bank y3;
    struct s5p_gpio_bank y4;
    struct s5p_gpio_bank y5;
    struct s5p_gpio_bank y6;
    struct s5p_gpio_bank res1[0x3];
    struct s5p_gpio_bank c4;
    struct s5p_gpio_bank res2[0x48];
    struct s5p_gpio_bank x0;
    struct s5p_gpio_bank x1;
    struct s5p_gpio_bank x2;
    struct s5p_gpio_bank x3;
};

struct exynos5_gpio_part2 {
    struct s5p_gpio_bank e0;
    struct s5p_gpio_bank e1;
    struct s5p_gpio_bank f0;
    struct s5p_gpio_bank f1;
    struct s5p_gpio_bank g0;
    struct s5p_gpio_bank g1;
    struct s5p_gpio_bank g2;
    struct s5p_gpio_bank h0;
    struct s5p_gpio_bank h1;
};

struct exynos5_gpio_part3 {
    struct s5p_gpio_bank v0;
    struct s5p_gpio_bank v1;
    struct s5p_gpio_bank res1[0x1];
    struct s5p_gpio_bank v2;
    struct s5p_gpio_bank v3;
    struct s5p_gpio_bank res2[0x1];
    struct s5p_gpio_bank v4;
};

struct exynos5_gpio_part4 {
    struct s5p_gpio_bank z;
};

/* functions */
void s5p_gpio_cfg_pin(struct s5p_gpio_bank *bank, int gpio, int cfg);
void s5p_gpio_direction_output(struct s5p_gpio_bank *bank, int gpio, int en);
void s5p_gpio_direction_input(struct s5p_gpio_bank *bank, int gpio);
void s5p_gpio_set_value(struct s5p_gpio_bank *bank, int gpio, int en);
unsigned int s5p_gpio_get_value(struct s5p_gpio_bank *bank, int gpio);
void s5p_gpio_set_pull(struct s5p_gpio_bank *bank, int gpio, int mode);
void s5p_gpio_set_drv(struct s5p_gpio_bank *bank, int gpio, int mode);
void s5p_gpio_set_rate(struct s5p_gpio_bank *bank, int gpio, int mode);

/* GPIO pins per bank  */
#define GPIO_PER_BANK 8

#define exynos4_gpio_part1_get_nr(bank, pin) \
    ((((((unsigned int) &(((struct exynos4_gpio_part1 *) \
                   EXYNOS4_GPIO_PART1_BASE)->bank)) \
        - EXYNOS4_GPIO_PART1_BASE) / sizeof(struct s5p_gpio_bank)) \
      * GPIO_PER_BANK) + pin)

#define EXYNOS4_GPIO_PART1_MAX ((sizeof(struct exynos4_gpio_part1) \
                / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos4_gpio_part2_get_nr(bank, pin) \
    (((((((unsigned int) &(((struct exynos4_gpio_part2 *) \
                EXYNOS4_GPIO_PART2_BASE)->bank)) \
        - EXYNOS4_GPIO_PART2_BASE) / sizeof(struct s5p_gpio_bank)) \
      * GPIO_PER_BANK) + pin) + EXYNOS4_GPIO_PART1_MAX)

#define exynos5_gpio_part1_get_nr(bank, pin) \
    ((((((unsigned int) &(((struct exynos5_gpio_part1 *) \
                   EXYNOS5_GPIO_PART1_BASE)->bank)) \
        - EXYNOS5_GPIO_PART1_BASE) / sizeof(struct s5p_gpio_bank)) \
      * GPIO_PER_BANK) + pin)

#define EXYNOS5_GPIO_PART1_MAX ((sizeof(struct exynos5_gpio_part1) \
                / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos5_gpio_part2_get_nr(bank, pin) \
    (((((((unsigned int) &(((struct exynos5_gpio_part2 *) \
                EXYNOS5_GPIO_PART2_BASE)->bank)) \
        - EXYNOS5_GPIO_PART2_BASE) / sizeof(struct s5p_gpio_bank)) \
      * GPIO_PER_BANK) + pin) + EXYNOS5_GPIO_PART1_MAX)

#define EXYNOS5_GPIO_PART2_MAX ((sizeof(struct exynos5_gpio_part2) \
                / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos5_gpio_part3_get_nr(bank, pin) \
    (((((((unsigned int) &(((struct exynos5_gpio_part3 *) \
                EXYNOS5_GPIO_PART3_BASE)->bank)) \
        - EXYNOS5_GPIO_PART3_BASE) / sizeof(struct s5p_gpio_bank)) \
      * GPIO_PER_BANK) + pin) + EXYNOS5_GPIO_PART2_MAX)

static inline unsigned int s5p_gpio_base(int nr)
{
        if (nr < EXYNOS5_GPIO_PART1_MAX)
            return EXYNOS5_GPIO_PART1_BASE;
        else if (nr < EXYNOS5_GPIO_PART2_MAX)
            return EXYNOS5_GPIO_PART2_BASE;
        else
            return EXYNOS5_GPIO_PART3_BASE;

    return 0;
}

static inline unsigned int s5p_gpio_part_max(int nr)
{
        if (nr < EXYNOS5_GPIO_PART1_MAX)
            return 0;
        else if (nr < EXYNOS5_GPIO_PART2_MAX)
            return EXYNOS5_GPIO_PART1_MAX;
        else
            return EXYNOS5_GPIO_PART2_MAX;

    return 0;
}

/* Pin configurations */
#define GPIO_INPUT  0x0
#define GPIO_OUTPUT 0x1
#define GPIO_IRQ    0xf
#define GPIO_FUNC(x)    (x)

/* Pull mode */
#define GPIO_PULL_NONE  0x0
#define GPIO_PULL_DOWN  0x1
#define GPIO_PULL_UP    0x3

/* Drive Strength level */
#define GPIO_DRV_1X 0x0
#define GPIO_DRV_3X 0x1
#define GPIO_DRV_2X 0x2
#define GPIO_DRV_4X 0x3
#define GPIO_DRV_FAST   0x0
#define GPIO_DRV_SLOW   0x1


#endif
