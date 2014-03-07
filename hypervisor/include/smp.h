#ifndef __SMP_H__
#define __SMP_H__

#include "arch_types.h"
#include "armv7_p15.h"

#define MPIDR_MASK 0xFFFFFF

static inline uint32_t smp_processor_id(void)
{
    /* Cortex-A15,
     * MPIDR[1:0] - CPUID - 0,1, 2, or 3
     * MPIDR[7:2] - Reserved, Read as zero
     */
    return read_mpidr() & MPIDR_MASK;
}

#endif
