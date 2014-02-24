#include "mct_priv.h"
#include <io-exynos.h>
#define likely(x)   __builtin_expect(!!(x), 1)
static void *exynos5_sys_timer;
static inline uint32_t exynos5_mct_read(uint32_t offset)
{
    return vmm_readl((void *) EXYNOS5_MCT_BASE + offset);
}
static void exynos5_mct_write(uint32_t value, uint32_t offset)
{
    uint32_t stat_addr;
    uint32_t mask;
    uint32_t i;
    exynos5_sys_timer = (void *)EXYNOS5_MCT_BASE;
    vmm_writel(value, exynos5_sys_timer + offset);
    if (likely(offset >= EXYNOS5_MCT_L_BASE(0))) {
        uint32_t base = offset & EXYNOS5_MCT_L_MASK;
        switch (offset & ~EXYNOS5_MCT_L_MASK) {
            case (uint32_t) MCT_L_TCON_OFFSET:
                stat_addr = base + MCT_L_WSTAT_OFFSET;
                mask = 1 << 3;
                break;
            case (uint32_t) MCT_L_ICNTB_OFFSET:
                stat_addr = base + MCT_L_WSTAT_OFFSET;
                mask = 1 << 1;  /* L_ICNTB write status */
                break;
            case (uint32_t) MCT_L_TCNTB_OFFSET:
                stat_addr = base + MCT_L_WSTAT_OFFSET;
                mask = 1 << 0;  /* L_TCNTB write status */
                break;
            default:
                return;
        }
    } else {
        switch (offset) {
            case (uint32_t) EXYNOS5_MCT_G_TCON:
                stat_addr = EXYNOS5_MCT_G_WSTAT;
                mask = 1 << 16; /* G_TCON write status */
                break;
            case (uint32_t) EXYNOS5_MCT_G_COMP0_L:
                stat_addr = EXYNOS5_MCT_G_WSTAT;
                mask = 1 << 0;  /* G_COMP0_L write status */
                break;
            case (uint32_t) EXYNOS5_MCT_G_COMP0_U:
                stat_addr = EXYNOS5_MCT_G_WSTAT;
                mask = 1 << 1;  /* G_COMP0_U write status */
                break;
            case (uint32_t) EXYNOS5_MCT_G_COMP0_ADD_INCR:
                stat_addr = EXYNOS5_MCT_G_WSTAT;
                mask = 1 << 2;  /* G_COMP0_ADD_INCR w status */
                break;
            case (uint32_t) EXYNOS5_MCT_G_CNT_L:
                stat_addr = EXYNOS5_MCT_G_CNT_WSTAT;
                mask = 1 << 0;  /* G_CNT_L write status */
                break;
            case (uint32_t) EXYNOS5_MCT_G_CNT_U:
                stat_addr = EXYNOS5_MCT_G_CNT_WSTAT;
                mask = 1 << 1;  /* G_CNT_U write status */
                break;
            default:
                return;
        }
    }
    /* Wait maximum 1 ms until written values are applied */
    for (i = 0; i < 0x1000; i++) {
        /* So we do this loop up to 1000 times */
        if (exynos5_mct_read(stat_addr) & mask) {
            vmm_writel(mask, exynos5_sys_timer + stat_addr);
            return;
        }
    }
}

void mct_init(void)
{
    uint32_t reg;
    exynos5_mct_write(0, EXYNOS5_MCT_G_CNT_L);
    exynos5_mct_write(0, EXYNOS5_MCT_G_CNT_U);
    reg = exynos5_mct_read(EXYNOS5_MCT_G_TCON);
    reg |= MCT_G_TCON_START;
    exynos5_mct_write(reg, EXYNOS5_MCT_G_TCON);
}
