#ifndef _MONITORING_H__
#define _MONITORING_H__

#include <log/uart_print.h>
#include <hvmm_types.h>
#include <arch_types.h>

#define NUM_BREAK_POINT 50
#define NUM_DI (NUM_BREAK_POINT * 2)
uint32_t inst[NUM_DI][3];
/*
* EMPTY =  0b000
* TRAP =   0b001
* RETRAP = 0b010
* BREAK =  0b100
*/
enum breakpoint_type {
    EMPTY =      0b000,
    TRAP =       0b001,
    RETRAP =     0b010,
    BREAK_TRAP = 0b100
};
enum ttbr_type {
    TTBR0,
    TTBR1
};

enum inst_index {
    INST,
    INST_VA,
    INST_TYPE
};

/*
 * DI, inject hvc at VA(PA)
 */
hvmm_status_t set_monitor_point(uint32_t va);

/*
 * DI, inject ori inst at VA(PA)
 */
hvmm_status_t clean_monitor_point(uint32_t va);

uint64_t va_to_pa(uint32_t va, uint32_t ttbr_num);

uint32_t load_inst(uint32_t va);
uint32_t inst_type(uint32_t va);
uint32_t store_inst(uint32_t va, uint32_t type);
uint32_t clean_inst(uint32_t va, uint32_t type);
void show_symbole(uint32_t va);
extern uint32_t system_map_start;
extern uint32_t system_map_end;

#endif
