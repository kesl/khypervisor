#ifndef __ARMV7_P15_H__
#define __ARMV7_P15_H__
#include "arch_types.h"


/* Generic ARM Registers */
#define read_cpsr()             ({ uint32_t rval; asm volatile(\
                                " mrs     %0, cpsr\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })
#define read_vbar()             ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c12, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })
#define write_vbar(val)         asm volatile(\
                                " mcr     p15, 0, %0, c12, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

/* ARMv7 Registers */
#define HCR_FMO     0x8
#define HCR_IMO     0x10
#define HCR_VI      (0x1 << 7)

/* 32bit case only */
#define read_ttbr0()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c2, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_ttbr0(val)        asm volatile(\
                                " mcr     p15, 0, %0, c2, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_ttbr1()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c2, c0, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_ttbr1(val)        asm volatile(\
                                " mcr     p15, 0, %0, c2, c0, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_ttbcr()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c2, c0, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_ttbcr(val)        asm volatile(\
                                " mcr     p15, 0, %0, c2, c0, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_mair0()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_mair0(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_mair1()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_mair1(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hmair0()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hmair0(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hmair1()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hmair1(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hsr()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c5, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })
#define read_htcr()             ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c2, c0, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_htcr(val)         asm volatile(\
                                " mcr     p15, 4, %0, c2, c0, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hsctlr()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c1, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hsctlr(val)       asm volatile(\
                                " mcr     p15, 4, %0, c1, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_sctlr()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c1, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_sctlr(val)       asm volatile(\
                                " mcr     p15, 0, %0, c1, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_httbr()            ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 4, %0, %1, c2\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_httbr(val)    asm volatile(\
                            " mcrr     p15, 4, %0, %1, c2\n\t" \
                            : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                            : "memory", "cc")

#define read_vtcr()             ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c2, c1, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_vtcr(val)         asm volatile(\
                                " mcr     p15, 4, %0, c2, c1, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_vttbr()            ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 6, %0, %1, c2\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_vttbr(val)    asm volatile(\
                            " mcrr     p15, 6, %0, %1, c2\n\t" \
                            : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                            : "memory", "cc")

#define read_hcr()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c1, c1, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hcr(val)          asm volatile(\
                                " mcr     p15, 4, %0, c1, c1, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_midr()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c0, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define read_mpidr()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c0, c0, 5\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define read_vmpidr()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c0, c0, 5\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_vmpidr(val)       asm volatile(\
                                " mcr     p15, 4, %0, c0, c0, 5\n\t" \
                                : : "r" ((val)) : "memory", "cc")

/* Generic Timer */

#define read_cntfrq()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntfrq(val)       asm volatile(\
                                " mcr     p15, 0, %0, c14, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cnthctl()          ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c14, c1, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cnthctl(val)      asm volatile(\
                                " mcr     p15, 4, %0, c14, c1, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cnthp_ctl()        ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c14, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cnthp_ctl(val)    asm volatile(\
                                " mcr     p15, 4, %0, c14, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cnthp_cval()       ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 6, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_cnthp_cval(val) asm volatile(\
                              " mcrr     p15, 6, %0, %1, c14\n\t" \
                              : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                              : "memory", "cc")

#define read_cnthp_tval()       ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c14, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cnthp_tval(val)   asm volatile(\
                                " mcr     p15, 4, %0, c14, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntkctl()          ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c1, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntkctl(val)      asm volatile(\
                                " mcr     p15, 0, %0, c14, c1, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntp_ctl()         ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntp_ctl(val)     asm volatile(\
                                " mcr     p15, 0, %0, c14, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntp_cval()        ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 2, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_cntp_cval(val)  asm volatile(\
                              " mcrr     p15, 2, %0, %1, c14\n\t" \
                              : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                              : "memory", "cc")

#define read_cntp_tval()        ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntp_tval(val)    asm volatile(\
                                " mcr     p15, 0, %0, c14, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntpct()           ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 0, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define read_cntv_ctl()         ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c3, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntv_ctl(val)     asm volatile(\
                                " mcr     p15, 0, %0, c14, c3, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntv_cval()        ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 3, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_cntv_cval(val)  asm volatile(\
                              " mcrr     p15, 3, %0, %1, c14\n\t" \
                              : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                              : "memory", "cc")

#define read_cntv_tval()        ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c3, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntv_tval(val)    asm volatile(\
                                " mcr     p15, 0, %0, c14, c3, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntvct()           ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 1, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define read_cntvoff()          ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 4, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_cntvoff(val)    asm volatile(\
                              " mcrr     p15, 4, %0, %1, c14\n\t" \
                              : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                              : "memory", "cc")

#define read_hdfar()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c6, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hdfar(val)        asm volatile(\
                                " mcr     p15, 4, %0, c6, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hifar()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c6, c0, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hifar(val)        asm volatile(\
                                " mcr     p15, 4, %0, c6, c0, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hpfar()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c6, c0, 4\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hpfar(val)        asm volatile(\
                                " mcr     p15, 4, %0, c6, c0, 4\n\t" \
                                : : "r" ((val)) : "memory", "cc")

/* TLB maintenance operations */

/* Invalidate entire unified TLB */
#define invalidate_unified_tlb(val)      asm volatile(\
                " mcr     p15, 0, %0, c8, c7, 0\n\t" \
                : : "r" ((val)) : "memory", "cc")
#endif


