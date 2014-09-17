#ifndef _MONITORING_H__
#define _MONITORING_H__

#include <log/uart_print.h>
#include <hvmm_types.h>
#include <arch_types.h>
#include <guest.h>

#define NUM_BREAK_POINT 50
#define NUM_DI (NUM_BREAK_POINT * 2)
#define HVC_TRAP 0xe14fff7c

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
#define MAX_LENGTH_SYMBOL 50
struct system_map {
    uint32_t address;
    uint8_t type;
    char symbol[MAX_LENGTH_SYMBOL];
};

uint64_t va_to_pa(uint32_t va, uint32_t ttbr_num);

uint32_t load_inst(uint32_t va);
uint32_t inst_type(uint32_t va);
uint32_t store_inst(uint32_t va, uint32_t type);
uint32_t clean_inst(uint32_t va, uint32_t type);
void show_symbol(uint32_t va);
extern uint32_t system_map_start;
extern uint32_t system_map_end;
void monitor_init(void);
int symbol_getter_from_va(uint32_t va, char *symbol);
hvmm_status_t kmo_list(void);
hvmm_status_t kmo_run(void);
hvmm_status_t kmo_break(uint32_t va, uint32_t type);
hvmm_status_t kmo_clean(uint32_t va, uint32_t type);
void kmo_break_handler(struct arch_regs **regs, uint32_t type);
#endif
