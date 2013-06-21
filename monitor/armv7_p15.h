#ifndef __ARMV7_P15_H__
#define __ARMV7_P15_H__
#include "arch_types.h"

/* Generic ARM Registers */
#define read_cpsr()              ({ uint32_t rval; asm volatile(\
                                " mrs     %0, cpsr\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})

/* ARMv7 Registers */
#define read_mair0()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})

#define write_mair0(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 0\n\t" \
                                :: "r" ((val)) : "memory", "cc")

#define read_mair1()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})

#define write_mair1(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 1\n\t" \
                                :: "r" ((val)) : "memory", "cc")

#define read_hmair0()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})

#define write_hmair0(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 0\n\t" \
                                :: "r" ((val)) : "memory", "cc")

#define read_hmair1()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})

#define write_hmair1(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 1\n\t" \
                                :: "r" ((val)) : "memory", "cc")

#define read_hsr()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c5, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})
#define read_htcr()             ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c2, c0, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})

#define write_htcr(val)         asm volatile(\
                                " mcr     p15, 4, %0, c2, c0, 2\n\t" \
                                :: "r" ((val)) : "memory", "cc")

#define read_hsctlr()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c1, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})

#define write_hsctlr(val)       asm volatile(\
                                " mcr     p15, 4, %0, c1, c0, 0\n\t" \
                                :: "r" ((val)) : "memory", "cc")

#define read_httbr()            ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 4, %0, %1, c2\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1);})

#define write_httbr(val)        asm volatile(\
                                " mcrr     p15, 4, %0, %1, c2\n\t" \
                                :: "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                                : "memory", "cc")

#define read_vtcr()             ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c2, c1, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})

#define write_vtcr(val)         asm volatile(\
                                " mcr     p15, 4, %0, c2, c1, 2\n\t" \
                                :: "r" ((val)) : "memory", "cc")

#define read_vttbr()            ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 6, %0, %1, c2\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1);})

#define write_vttbr(val)        asm volatile(\
                                " mcrr     p15, 6, %0, %1, c2\n\t" \
                                :: "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                                : "memory", "cc")

#define read_hcr()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c1, c1, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})

#define write_hcr(val)          asm volatile(\
                                " mcr     p15, 4, %0, c1, c1, 0\n\t" \
                                :: "r" ((val)) : "memory", "cc")

#endif

