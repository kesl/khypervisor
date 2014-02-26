#ifndef __VIRQ_H__
#define __VIRQ_H__
#include <hvmm_types.h>

hvmm_status_t virq_inject(vmid_t vmid, uint32_t virq,
        uint32_t pirq, uint8_t hw);
hvmm_status_t virq_init(void);
#endif
