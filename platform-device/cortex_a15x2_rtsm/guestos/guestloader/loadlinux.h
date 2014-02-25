#ifndef __LOADLINUX_H__
#define __LOADLINUX_H__

#include "arch_types.h"

void loadlinux_setup_tags( uint32_t *src );
void loadlinux_run_zImage( uint32_t start_addr );

struct arch_regs {
        uint32_t cpsr;
        uint32_t pc;
        uint32_t lr;
        uint32_t gpr[13];
} __attribute((packed));

#endif //__LOADLINUX_H__
