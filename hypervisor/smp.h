#ifndef __SMP_H__
#define __SMP_H__

#include "arch_types.h"
/**
 * @brief Return the smp processor id
 *
 * read the mpid from mpidr and return id<br>
 * - Coretex-A15<br>
 *   - MPIDR[1:0] - CPUID - 0, 1, 2, OR 3<br>
 *   - MPIDR[7:2] - Rsereved, Read as zero
 * @param void
 * @return uint32_t smp processor id
 */
uint32_t smp_processor_id(void);

#endif
