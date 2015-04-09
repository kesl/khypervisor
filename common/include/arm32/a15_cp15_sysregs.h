#ifndef __A15_CP15_SYSREGS_H_
#define __A15_CP15_SYSREGS_H_

#include "arch_types.h"

/*
 * Cortex-A15 Implementation Defined Registers
 * ACTLR
 * L2CTLR
 * L3ECTLR
 * IL1DATA0
 * IL1DATA1
 * IL1DATA2
 * DL1DATA0
 * DL1DATA1
 * DL1DATA2
 * DL1DATA3
 * RAMINDEX
 * L2ACTLR
 * L2PFR
 * CBAR - read_cbar() - Configuration Base Address Register
 * CPUMERRSR
 * L2MERRSR
 */

#define read_cbar()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c15, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })
#endif
