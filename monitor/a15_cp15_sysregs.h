#ifndef __A15_CP15_SYSREGS_H_
#define __A15_CP15_SYSREGS_H_

#include "arch_types.h"

/* Cortex-A15 */
/* a15_cp15_sysregs.h */
#define read_midr()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c0, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})
#define read_cbar()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c15, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval;})
#endif
