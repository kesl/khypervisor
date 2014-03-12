#ifndef __SMP_H__
#define __SMP_H__

#include "arch_types.h"
#include "armv7_p15.h"

#define MPIDR_MASK 0xFFFFFF

/**
 * @brief Return the smp processor id
 *
 * Read the mpid from mpidr and return id
 * - Coretex-A15<br>
 *   - MPIDR[1:0] - CPUID - 0, 1, 2, OR 3
 *   - MPIDR[7:2] - Rsereved, Read as zero
 * @param void
 * @return Smp processor id
 */
static inline uint32_t smp_processor_id(void)
{
    return read_mpidr() & MPIDR_MASK;
}

#endif
