#ifndef _MONITORING_H__
#define _MONITORING_H__

#include <log/uart_print.h>
#include <hvmm_types.h>
#include <arch_types.h>
#include <guest.h>

#define NUM_BREAK_POINT 50
#define NUM_DI (NUM_BREAK_POINT * 2)
#define HVC_TRAP 0xe14fff7c
#define MO_GUEST 1
#define MO_VIRQ 20

#define LIST 0
#define MONITORING 1
#define MEMORY 2

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
    INST = 0,
    INST_VA,
    INST_TYPE,
    NUM_INST
};

#define MONITOR_READ_LIST           0x08
#define MONITOR_READ_RUN            0x10
#define MONITOR_READ_CEAN_ALL       0x18
#define MONITOR_READ_DUMP_MEMORY    0x1C

#define MONITOR_WRITE_TRACE_GUEST           0x00
#define MONITOR_WRITE_CLEAN_TRACE_GUEST     0x04
#define MONITOR_WRITE_BREAK_GUEST           0x0C
#define MONITOR_WRITE_CLEAN_BREAK_GUEST     0x14

/* size 88 -> 0x8EC00100 : memory dump*/
struct monitoring_data {
    uint8_t type;
    uint32_t caller_va;
    uint32_t callee_va;
    uint32_t inst;
    uint32_t sp;
    struct arch_regs regs;
    uint32_t memory_range;
    uint32_t start_memory;
};

uint64_t va_to_pa(uint32_t va, uint32_t ttbr_num);
uint32_t monitor_load_inst(uint32_t va);
uint32_t monitor_inst_type(uint32_t va);
uint32_t monitor_store_inst(uint32_t va, uint32_t type);
uint32_t monitor_clean_inst(uint32_t va, uint32_t type);

hvmm_status_t monitor_run_guest(void);
hvmm_status_t monitor_break_guest(void);
hvmm_status_t monitor_insert_break_to_guest(uint32_t va);
hvmm_status_t monitor_insert_trace_to_guest(uint32_t va);
hvmm_status_t monitor_clean_guest(uint32_t va, uint32_t type);
hvmm_status_t monitor_clean_break_guest(uint32_t va);
hvmm_status_t monitor_clean_trace_guest(uint32_t va);
hvmm_status_t monitor_clean_all_guest();
hvmm_status_t monitor_dump_guest_memory(void);
hvmm_status_t monitor_detect_fault();
hvmm_status_t monitor_recovery_guest();
hvmm_status_t monitor_request(int irq, vmid_t vmid, int address);
hvmm_status_t monitor_notify_guest(vmid_t vmid);
hvmm_status_t monitor_list(void);
hvmm_status_t monitor_init();

#endif
