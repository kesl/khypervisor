/*
 * K-Hypervisor : Guest Monitoring Framework
 */

#include <k-hypervisor-config.h>
#include <interrupt.h>
#include <monitor.h>
#include <arch_types.h>
#include <log/print.h>
#include <log/uart_print.h>
#include <asm_io.h>
#include <armv7_p15.h>
#include <guest.h>

static uint32_t inst[NUM_GUESTS_STATIC][NUM_DI][NUM_INST];

/*
 * Monitoring point Manager : store_inst, load_inst, clean_inst
 */
uint32_t monitor_store_inst(vmid_t vmid, uint32_t va, enum breakpoint_type type)
{
    int i;
    uint32_t pa;

    for (i = 0; i < NUM_DI; i++) {
        if (inst[vmid][i][INST_VA] == va) {
            printH("Already set breakpoint %d\n", type);
            inst[vmid][i][INST_TYPE] |= type;
            return NOTFOUND;
        }
    }
    for (i = 0; i < NUM_DI; i++) {
        if (inst[vmid][i][INST] == EMPTY) {
            pa = va_to_pa(vmid, va, TTBR0);
            inst[vmid][i][INST] = (*(uint32_t *)pa);
            inst[vmid][i][INST_VA] = va;
            inst[vmid][i][INST_TYPE] = type;
            printh("[monitor_store_inst] : %x\n", inst[vmid][i][INST]);
            return FOUND;
        }
    }
    return NOTFOUND;
}

enum breakpoint_type monitor_inst_type(vmid_t vmid, uint32_t va)
{
    int i;

    for (i = 0; i < NUM_DI; i++)
        if (inst[vmid][i][INST_VA] == va)
            return inst[vmid][i][INST_TYPE];

    return NOTFOUND;
}

uint32_t monitor_clean_inst(vmid_t vmid, uint32_t va, enum breakpoint_type type)
{
    int i;
    for (i = 0; i < NUM_DI; i++) {
        if (inst[vmid][i][INST_VA] == va) {
            inst[vmid][i][INST_TYPE] &= ~(type);
            if (inst[vmid][i][INST_TYPE] == EMPTY) {
                inst[vmid][i][INST] = EMPTY;
                inst[vmid][i][INST_VA] = EMPTY;
                return FOUND;
            }
        break;
        }
    }
    printH("CLEAN NOTFOUND\n");
    return NOTFOUND;
}

uint32_t monitor_load_inst(vmid_t vmid, uint32_t va)
{
    int i, ori_inst;

    for (i = 0; i < NUM_DI; i++) {
        if (inst[vmid][i][INST_VA] == va) {
            ori_inst = inst[vmid][i][INST];
            return ori_inst;
        }
    }

    printH("[%s : %d] corresponded instruction not found va : %x\n",
            __func__, __LINE__, va);

    return NOTFOUND;
}

hvmm_status_t monitor_list(struct monitor_vmid *mvmid, uint32_t va)
{
    vmid_t vmid = mvmid->vmid_target;
    vmid_t vmid_monitor = mvmid->vmid_monitor;
    struct monitoring_data *data;
    uint32_t *dump_base = (uint32_t *)SHARED_DUMP_ADDRESS;
    int i, monitor_cnt = 0;

    for (i = 0; i < NUM_DI; i++) {
        if (inst[vmid][i][INST] != EMPTY) {
            /* saved va -> inst */
            *dump_base = inst[vmid][i][INST_VA];
            dump_base++;
            *dump_base = inst[vmid][i][INST];
            monitor_cnt++;
        }
    }
    if (monitor_cnt > 0) {
        data = (struct monitoring_data *)(SHARED_ADDRESS);
        data->type = LIST;
        data->monitor_cnt = monitor_cnt;
        monitor_notify_guest(vmid_monitor);
        flush_dcache_all();
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_break_guest(vmid_t vmid)
{
    /* Run other guest for stop this guest */
    set_manually_select_vmid(1);
    guest_switchto(1, 0);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_run_guest(struct monitor_vmid *mvmid, uint32_t va)
{
    clean_manually_select_vmid();
    guest_switchto(0, 0);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_insert_break_to_guest(struct monitor_vmid *mvmid,
                                                uint32_t va)
{
    vmid_t vmid = mvmid->vmid_target;


    if (NOTFOUND != monitor_store_inst(vmid, va, MONITOR_BREAK_TRAP)) {
        /* TODO : This code will move to hardware interface */
        writel(HVC_TRAP, (uint32_t)va_to_pa(vmid, va, TTBR0));
        flush_dcache_all();
        invalidate_icache_all();
    }
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_insert_trace_to_guest(struct monitor_vmid *mvmid,
                                                uint32_t va)
{
    vmid_t vmid = mvmid->vmid_target;
    if (monitor_store_inst(vmid, va, MONITOR_TRACE_TRAP)) {
        /* TODO : This code will move to hardware interface */
        writel(HVC_TRAP, (uint32_t)va_to_pa(vmid, va, TTBR0));
        flush_dcache_all();
        invalidate_icache_all();
        return HVMM_STATUS_SUCCESS;
    }
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_clean_guest(struct monitor_vmid *mvmid, uint32_t va,
                                    uint32_t type)
{
    uint32_t inst;
    vmid_t vmid = mvmid->vmid_target;
    inst = monitor_load_inst(vmid, va);
    if (inst != NOTFOUND && monitor_clean_inst(vmid, va, type)) {
        /* TODO : This code will move to hardware interface */
        printH("[Monitor device] : clean va %x\n", va);
        writel(inst, (uint32_t)va_to_pa(vmid, va , TTBR0));
    }

    /* Clean point's retrap point */
    inst = monitor_load_inst(vmid, (va) + 4);
    if (inst != NOTFOUND && monitor_clean_inst(vmid, (va) + 4,
                MONITOR_RETRAP)) {
        /* TODO : This code will move to hardware interface */
        writel(inst, (uint32_t)va_to_pa(vmid, (va) + 4 , TTBR0));
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_clean_break_guest(struct monitor_vmid *mvmid, uint32_t va)
{
    hvmm_status_t result;
    result = monitor_clean_guest(mvmid, va, MONITOR_BREAK_TRAP);
    flush_dcache_all();
    invalidate_icache_all();
    return result;
}

hvmm_status_t monitor_clean_trace_guest(struct monitor_vmid *mvmid, uint32_t va)
{
    hvmm_status_t result;
    result = monitor_clean_guest(mvmid, va, MONITOR_TRACE_TRAP);
    flush_dcache_all();
    invalidate_icache_all();
    return result;
}

hvmm_status_t monitor_clean_all_guest(struct monitor_vmid *mvmid, uint32_t va)
{
    int i;
    vmid_t vmid = mvmid->vmid_target;
    printH("[Monitor device] : monitor_clean_all_guest\n");
    for (i = 0; i < NUM_DI; i++) {
        if (inst[vmid][i][INST] != EMPTY) {
            monitor_clean_guest(mvmid, inst[vmid][i][INST_VA],
                    MONITOR_TRACE_TRAP | MONITOR_RETRAP | MONITOR_BREAK_TRAP);
        }
    }

    flush_dcache_all();
    invalidate_icache_all();

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_dump_guest_memory(struct monitor_vmid *mvmid, uint32_t va)
{
    uint32_t range, base_memory;
    uint32_t *dump_base, *base_memory_pa;
    struct monitoring_data *data;
    vmid_t vmid, vmid_monitor;
    int i;

    dump_base = (uint32_t *)SHARED_DUMP_ADDRESS;
    vmid = mvmid->vmid_target;
    vmid_monitor = mvmid->vmid_monitor;
    data =  (struct monitoring_data *)(SHARED_ADDRESS);

    range = data->memory_range;
    base_memory = data->start_memory;

    for (i = 0; i < range; i++) {
        base_memory_pa = (uint32_t *)(uint32_t)va_to_pa(vmid, base_memory, 0);
        *dump_base = *base_memory_pa;
        dump_base++;
        base_memory += 4;
    }

    if (range > 0) {
        data->type = MEMORY;
        monitor_notify_guest(vmid_monitor);
        flush_dcache_all();
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_stop(struct monitor_vmid *mvmid, uint32_t va)
{
    monitor_break_guest(mvmid->vmid_target);
}
hvmm_status_t monitor_register(struct monitor_vmid *mvmid, uint32_t va)
{
    hvmm_status_t ret = HVMM_STATUS_SUCCESS;
    vmid_t vmid, vmid_monitor;
    vmid = mvmid->vmid_target;
    vmid_monitor = mvmid->vmid_monitor;
    //struct arch_regs *regs;
    struct monitoring_data *data;
    data = (struct monitoring_data *)(SHARED_ADDRESS);
    data->type = REGISTER;

    guest_copy(&(data->guest_info), vmid);

    monitor_notify_guest(vmid_monitor);
    flush_dcache_all();
    return ret;
}
hvmm_status_t monitor_reboot_guest(struct monitor_vmid *mvmid)
{
    hvmm_status_t ret = HVMM_STATUS_SUCCESS;

    monitor_clean_all_guest(mvmid, 0);

    /* TODO there is dependency target board problem*/
    /* reboot_guest(mvmid, 0xB0000000, 0); */
    reboot_guest(mvmid->vmid_target, 0x70000000, 0);

    return ret;
}

hvmm_status_t monitor_reboot(struct monitor_vmid *mvmid, uint32_t va)
{
    hvmm_status_t ret = HVMM_STATUS_SUCCESS;

    monitor_reboot_guest(mvmid);

    return ret;
}

hvmm_status_t monitor_detect_fault(struct monitor_vmid *mvmid, uint32_t va)
{
    hvmm_status_t ret = HVMM_STATUS_SUCCESS;

    /* Trap to panic */
    /* monitor_insert_trace_to_guest(mvmid, 0x8015fbcc); */
    monitor_insert_trace_to_guest(mvmid, 0x407956c8);

#if 0
    /* TODO : It is working in hypervisor, not guest finally.. */
    if (va == 0x8015fbcc) {
        printH("Target guest's panic occured!\n");
        printH("Auto system recovery start...\n");
        monitor_recovery_guest(mvmid);
        reboot_guest(mvmid, 0xB0000000, regs);
        printH("regs is %x\n", (*regs)->pc);
    }
#endif
    return ret;
}

hvmm_status_t monitor_write_memory(struct monitor_vmid *mvmid, uint32_t va)
{
    uint32_t range, base_memory;
    uint8_t *dump_base;
    struct monitoring_data *data;
    vmid_t vmid, vmid_monitor;
    int i;

    flush_dcache_all();

    dump_base = (uint8_t *)SHARED_DUMP_ADDRESS;
    vmid = mvmid->vmid_target;
    vmid_monitor = mvmid->vmid_monitor;
    data =  (struct monitoring_data *)(SHARED_ADDRESS);

    range = data->memory_range;
    base_memory = data->start_memory;

    for (i = 0; i < range; i++) {
        *(uint8_t *)(uint8_t)va_to_pa(vmid, base_memory, 0) = *dump_base;
//        *dump_base = *(uint8_t *)(uint8_t)va_to_pa(vmid, base_memory, 0);
        dump_base++;
        base_memory++;
    }

    return HVMM_STATUS_SUCCESS;

}

hvmm_status_t monitor_recovery(struct monitor_vmid *mvmid, uint32_t va)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    return ret;
}
hvmm_status_t monitor_recovery_guest(struct monitor_vmid *mvmid)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    return ret;
}

hvmm_status_t monitor_request(int irq, struct monitor_vmid *mvmid, int address)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    return ret;
}

hvmm_status_t monitor_notify_guest(vmid_t vmid)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    interrupt_guest_inject(vmid, MONITOR_VIRQ, 0, INJECT_SW);

    return ret;
}

hvmm_status_t monitor_save(struct monitor_vmid *mvmid)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;


    return ret;
}

hvmm_status_t monitor_restore(struct monitor_vmid *mvmid)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    return ret;
}

hvmm_status_t monitor_init(void)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    return ret;
}
