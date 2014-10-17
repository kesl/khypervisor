#ifndef _VCPU_H__
#define _VCPU_H__

#include <guest_hw.h>

struct vcpu_struct {
    struct arch_regs regs;
    struct arch_context context;
    uint32_t vmpidr;
};

#endif
