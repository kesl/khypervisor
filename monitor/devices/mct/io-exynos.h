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
static inline uint32_t vmm_readl(volatile void *addr)
{     
     return arch_in_le32(addr);
} 
static inline void vmm_writel(uint32_t data, volatile void *addr)
{
     arch_out_le32(addr, data);
}
#endif
