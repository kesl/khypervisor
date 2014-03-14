#ifndef __SMP_H__
#define __SMP_H__

#include "arch_types.h"
#include "armv7_p15.h"

#define MPIDR_MASK 0xFFFFFF

/**
 * @brief Return the smp processor id
 *
 * Read the processor id from Multiprocessor ID Register(MPIDR) and return it.
 * - Coretex-A15
 *   - MPIDR[1:0] - CPUID - 0, 1, 2, OR 3
 *   - MPIDR[7:2] - Rsereved, Read as zero
 * @return Smp processor id.
 */
static inline uint32_t smp_processor_id(void)
{
    return read_mpidr() & MPIDR_MASK;
}

#endif
