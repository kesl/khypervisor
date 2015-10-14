#ifndef _MONITORING_H__
#define _MONITORING_H__

#include <log/uart_print.h>
#include <hvmm_types.h>
#include <arch_types.h>
#include <guest.h>
#include <guest_hw.h>

#define NUM_BREAK_POINT 50
#define NUM_DI (NUM_BREAK_POINT * 2)
#define HVC_TRAP 0xe14fff7c

#define LIST 0
#define MONITORING 1
#define MEMORY 2
#define REGISTER 3
#define BREAK 4

#define NOTFOUND 0
#define FOUND 1

/*
* EMPTY =  0b000
* TRAP =   0b001
* RETRAP = 0b010
* BREAK =  0b100
*/
enum breakpoint_type {
    EMPTY =      0b000,
    MONITOR_TRACE_TRAP =       0b001,
    MONITOR_RETRAP =     0b010,
    MONITOR_BREAK_TRAP = 0b100
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

#define MONITOR_READ_LIST           0x00
#define MONITOR_READ_RUN            0x01
#define MONITOR_READ_CEAN_ALL       0x02
#define MONITOR_READ_DUMP_MEMORY    0x03

#define MONITOR_WRITE_TRACE_GUEST           0x04
#define MONITOR_WRITE_CLEAN_TRACE_GUEST     0x05
#define MONITOR_WRITE_BREAK_GUEST           0x06
#define MONITOR_WRITE_CLEAN_BREAK_GUEST     0x07

/* 0xEC00100 : memory dump, 0xEC000D0 : vmid info*/
struct monitoring_data {
    uint8_t type;
    uint32_t caller_va;
    uint32_t callee_va;
    uint32_t inst;
    uint32_t memory_range;
    uint32_t start_memory;
    uint8_t monitor_cnt;
    struct vcpu guest_info;
};

struct monitor_vmid {
    vmid_t vmid_monitor;
    vmid_t vmid_target;
};

uint64_t va_to_pa(vmid_t vmid, uint32_t va, uint32_t ttbr_num);
void invalidate_icache_all(void);
void invalidate_dcache_all(void);
void flush_dcache_all(void);
void flush_cache(unsigned long start, unsigned long size);
uint32_t monitor_load_inst(vmid_t vmid, uint32_t va);
uint32_t monitor_inst_type(vmid_t vmid, uint32_t va);
uint32_t monitor_store_inst(vmid_t vmid, uint32_t va, uint32_t type);
uint32_t monitor_clean_inst(vmid_t vmid, uint32_t va, uint32_t type);

hvmm_status_t monitor_run_guest(struct monitor_vmid *mvmid, uint32_t va);
hvmm_status_t monitor_break_guest(vmid_t vmid);
hvmm_status_t monitor_insert_break_to_guest(struct monitor_vmid *mvmid,
                                                uint32_t va);
hvmm_status_t monitor_insert_trace_to_guest(struct monitor_vmid *mvmid,
                                                uint32_t va);
hvmm_status_t monitor_clean_guest(struct monitor_vmid *mvmid, uint32_t va,
                                                uint32_t type);
hvmm_status_t monitor_clean_break_guest(struct monitor_vmid *mvmid,
                                                uint32_t va);
hvmm_status_t monitor_clean_trace_guest(struct monitor_vmid *mvmid,
                                                uint32_t va);
hvmm_status_t monitor_clean_all_guest(struct monitor_vmid *mvmid, uint32_t va);
hvmm_status_t monitor_dump_guest_memory(struct monitor_vmid *mvmid,
                                                uint32_t va);
hvmm_status_t monitor_detect_fault(struct monitor_vmid *mvmid, uint32_t va);
hvmm_status_t monitor_register(struct monitor_vmid *mvmid, uint32_t va);
hvmm_status_t monitor_stop(struct monitor_vmid *mvmid, uint32_t va);
hvmm_status_t monitor_write_memory(struct monitor_vmid *mvmid, uint32_t va);
hvmm_status_t monitor_recovery_guest(struct monitor_vmid *mvmid);
hvmm_status_t monitor_request(int irq, struct monitor_vmid *mvmid, int address);
hvmm_status_t monitor_notify_guest(vmid_t vmid);
hvmm_status_t monitor_list(struct monitor_vmid *mvmid, uint32_t va);
hvmm_status_t monitor_reboot(struct monitor_vmid *mvmid, uint32_t va);
hvmm_status_t monitor_init(void);
hvmm_status_t monitor_recovery(struct monitor_vmid *mvmid, uint32_t va);
hvmm_status_t monitor_check_status(struct monitor_vmid *mvmid, uint32_t va);
#endif
