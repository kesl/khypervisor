#ifndef __SP804_H_
#define __SP804_H_
#include "arch_types.h"

uint32_t sp804_secondary_base(uint32_t sp804_base);
void sp804_irq_clear(uint32_t sp804_base);
void sp804_start(uint32_t sp804_base);
void sp804_load(uint32_t loadval, uint32_t sp804_base);
uint32_t sp804_read(uint32_t sp804_base);
void sp804_stop(uint32_t sp804_base);
void sp804_init_periodic(uint32_t sp804_base, uint32_t load_value);
void sp804_init_oneshot(uint32_t sp804_base);
void sp804_init(uint32_t sp804_base, uint32_t load_value);
hvmm_status_t hvmm_tests_sp804_timer(void);

#endif


