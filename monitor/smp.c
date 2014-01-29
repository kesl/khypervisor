
#include "armv7_p15.h"
#include "smp.h"

uint32_t smp_processor_id(void)
{
    /* Cortex-A15,
     * MPIDR[1:0] - CPUID - 0,1, 2, or 3
     * MPIDR[7:2] - Reserved, Read as zero
     */
    uint32_t id = read_mpidr() & 0x0FF;
    return id;
}
