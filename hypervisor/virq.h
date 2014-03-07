#ifndef __VIRQ_H__
#define __VIRQ_H__
#include <hvmm_types.h>

hvmm_status_t virq_inject(vmid_t vmid, uint32_t virq,
        uint32_t pirq, uint8_t hw);
/**
 * @brief   Initializes virq_entry structure and
            Sets callback function about injection of queued VIRQs.
 * @return  Always returns "success".
 */
hvmm_status_t virq_init(void);
#endif
