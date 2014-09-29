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
            inst[vmid][i][INST_TYPE] |= type;
            printH("Already set breakpoint\n");
            return 0;
        }
    }
    for (i = 0; i < NUM_DI; i++) {
        if (inst[vmid][i][INST] == EMPTY) {
            pa = va_to_pa(vmid, va, TTBR0);
            inst[vmid][i][INST] = (*(uint32_t *)pa);
            inst[vmid][i][INST_VA] = va;
            inst[vmid][i][INST_TYPE] = type;
            return 0;
        }
    }
    return 0;
}

enum breakpoint_type monitor_inst_type(vmid_t vmid, uint32_t va)
{
    int i;

    for (i = 0; i < NUM_DI; i++)
        if (inst[vmid][i][INST_VA] == va)
            return inst[vmid][i][INST_TYPE];

    return 0;
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
                return 1;
            }
        break;
        }
    }

    return 0;
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
    printh("[%s : %d] corresponded instruction not found\n",
            __func__, __LINE__);

    return 0;
}

hvmm_status_t monitor_list(struct monitor_vmid *mvmid, uint32_t va)
{
    int i;
    vmid_t vmid = mvmid->vmid_target;
    vmid_t vmid_monitor = mvmid->vmid_monitor;
    struct monitoring_data *data;

    for (i = 0; i < NUM_DI; i++) {
        if (inst[vmid][i][INST] != EMPTY) {
            data = (struct monitoring_data *)(SHARED_ADDRESS);
            data->type = LIST;
            data->caller_va = inst[vmid][i][INST_VA];
            data->inst = inst[vmid][i][INST];
            monitor_notify_guest(vmid_monitor);
        }
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

    monitor_store_inst(vmid, va, MONITOR_BREAK_TRAP);
    /* TODO : This code will move to hardware interface */
    writel(HVC_TRAP, (uint32_t)va_to_pa(vmid, va, TTBR0));

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_insert_trace_to_guest(struct monitor_vmid *mvmid,
                                                uint32_t va)
{
    vmid_t vmid = mvmid->vmid_target;

    monitor_store_inst(vmid, va, MONITOR_TRACE_TRAP);
    /* TODO : This code will move to hardware interface */
    writel(HVC_TRAP, (uint32_t)va_to_pa(vmid, va, TTBR0));

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_clean_guest(struct monitor_vmid *mvmid, uint32_t va,
                                    uint32_t type)
{
    uint32_t inst;
    vmid_t vmid = mvmid->vmid_target;

    inst = monitor_load_inst(vmid, va);
    if (monitor_clean_inst(vmid, va, type)) {
        /* TODO : This code will move to hardware interface */
        writel(inst, (uint32_t)va_to_pa(vmid, va , TTBR0));
    }

    /* Clean point's retrap point */
    inst = monitor_load_inst(vmid, (va) + 4);
    if (monitor_clean_inst(vmid, (va) + 4 , MONITOR_RETRAP)) {
        /* TODO : This code will move to hardware interface */
        writel(inst, (uint32_t)va_to_pa(vmid, (va) + 4 , TTBR0));
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_clean_break_guest(struct monitor_vmid *mvmid, uint32_t va)
{
    return monitor_clean_guest(mvmid, va, MONITOR_BREAK_TRAP);
}

hvmm_status_t monitor_clean_trace_guest(struct monitor_vmid *mvmid, uint32_t va)
{
    return monitor_clean_guest(mvmid, va, MONITOR_TRACE_TRAP);
}

hvmm_status_t monitor_clean_all_guest(struct monitor_vmid *mvmid, uint32_t va)
{
    int i;
    vmid_t vmid = mvmid->vmid_target;

    for (i = 0; i < NUM_DI; i++) {
        if (inst[vmid][i][INST] != EMPTY)
            monitor_clean_guest(mvmid,
                    inst[vmid][i][INST_VA],
                    MONITOR_TRACE_TRAP | MONITOR_RETRAP | MONITOR_BREAK_TRAP);
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_dump_guest_memory(struct monitor_vmid *mvmid, uint32_t va)
{
    uint32_t range, base_memory;
    uint32_t *dump_base = (uint32_t *)SHARED_DUMP_ADDRESS;
    struct monitoring_data *data =
        (struct monitoring_data *)(SHARED_ADDRESS);
    int i;
    uint32_t *base_memory_pa;
    vmid_t vmid = mvmid->vmid_target;
    vmid_t vmid_monitor = mvmid->vmid_monitor;

    range = data->memory_range;
    base_memory = data->start_memory;

    for (i = 0; i < range; i++) {
        base_memory_pa = (uint32_t *)(uint32_t)va_to_pa(vmid, base_memory, 0);
        *dump_base = *base_memory_pa;
        dump_base++;
        base_memory += 4;
    }
    data->type = MEMORY;
    monitor_notify_guest(vmid_monitor);

    return HVMM_STATUS_SUCCESS;
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

    /* Trap to loop_delay */
    monitor_insert_trace_to_guest(mvmid, 0x8015fbcc);

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

hvmm_status_t monitor_reboot_guest(struct monitor_vmid *mvmid)
{
    hvmm_status_t ret = HVMM_STATUS_SUCCESS;

    reboot_guest(mvmid, 0xB0000000, 0);

    return ret;
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
