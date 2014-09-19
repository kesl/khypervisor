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

uint32_t inst[NUM_DI][NUM_INST];

/*
 * Monitoring point Manager : store_inst, load_inst, clean_inst
 */
uint32_t monitor_store_inst(uint32_t va, enum breakpoint_type type)
{
    int i;
    uint32_t pa;
    for (i = 0; i < NUM_DI; i++) {
        if (inst[i][INST_VA] == va) {
            inst[i][INST_TYPE] |= type;
            printH("Already set breakpoint\n");
            return 0;
        }
    }
    for (i = 0; i < NUM_DI; i++) {
        if (inst[i][INST] == EMPTY) {
            pa = va_to_pa(va, TTBR0);
            inst[i][INST] = (*(uint32_t *)pa);
            inst[i][INST_VA] = va;
            inst[i][INST_TYPE] = type;
            return 0;
        }
    }
    return 0;
}

enum breakpoint_type monitor_inst_type(uint32_t va)
{
    int i;
    for (i = 0; i < NUM_DI; i++)
        if (inst[i][INST_VA] == va)
            return inst[i][INST_TYPE];
    return 0;
}

uint32_t monitor_clean_inst(uint32_t va, enum breakpoint_type type)
{
    int i;
    for (i = 0; i < NUM_DI; i++) {
        if (inst[i][INST_VA] == va) {
            inst[i][INST_TYPE] &= ~(type);
            if (inst[i][INST_TYPE] == EMPTY) {
                inst[i][INST] = EMPTY;
                inst[i][INST_VA] = EMPTY;
                return 1;
            }
        break;
        }
    }
    return 0;
}

uint32_t monitor_load_inst(uint32_t va)
{
    int i, ori_inst;
    for (i = 0; i < NUM_DI; i++) {
        if (inst[i][INST_VA] == va) {
            ori_inst = inst[i][INST];
            return ori_inst;
        }
    }
    printh("[%s : %d] corresponded instruction not found\n",
            __func__, __LINE__);
    return 0;
}

hvmm_status_t monitor_list(void)
{
    int i;
    struct monitoring_data *data;
    for (i = 0; i < NUM_DI; i++) {
        if (inst[i][INST] != EMPTY) {
            data = (struct monitoring_data *)(SHARED_ADDRESS);
            data->type = LIST;
            data->caller_va = inst[i][INST_VA];
            data->inst = inst[i][INST];
            monitor_notify_guest(MO_GUEST);
        }
    }
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_run_guest(void)
{
    clean_manually_select_vmid();
    guest_switchto(0, 0);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_break_guest(void)
{
    /* Run other guest for stop this guest */
    set_manually_select_vmid(1);
    guest_switchto(1, 0);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_insert_break_to_guest(uint32_t va)
{
    monitor_store_inst(va, BREAK_TRAP);
    /* TODO : This code will move to hardware interface */
    writel(HVC_TRAP, (uint32_t)va_to_pa(va, TTBR0));

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_insert_trace_to_guest(uint32_t va)
{
    monitor_store_inst(va, TRAP);
    /* TODO : This code will move to hardware interface */
    writel(HVC_TRAP, (uint32_t)va_to_pa(va, TTBR0));

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_clean_guest(uint32_t va, uint32_t type)
{
    uint32_t inst;

    inst = monitor_load_inst(va);
    if (monitor_clean_inst(va, type)) {
        /* TODO : This code will move to hardware interface */
        writel(inst, (uint32_t)va_to_pa(va , TTBR0));
    }

    /* Clean point's retrap point */
    inst = monitor_load_inst((va) + 4);
    if (monitor_clean_inst((va) + 4 , RETRAP)) {
        /* TODO : This code will move to hardware interface */
        writel(inst, (uint32_t)va_to_pa((va) + 4 , TTBR0));
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_clean_break_guest(uint32_t va)
{
    return monitor_clean_guest(va, BREAK_TRAP);
}

hvmm_status_t monitor_clean_trace_guest(uint32_t va)
{
    return monitor_clean_guest(va, TRAP);
}

hvmm_status_t monitor_clean_all_guest()
{
    int i;

    for (i = 0; i < NUM_DI; i++) {
        if (inst[i][INST] != EMPTY)
            monitor_clean_guest(inst[i][INST_VA], TRAP | RETRAP | BREAK_TRAP);
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_dump_guest_memory(void)
{
    uint32_t range, base_memory;
    uint32_t *dump_base = (uint32_t *)SHARED_DUMP_ADDRESS;
    struct monitoring_data *data =
        (struct monitoring_data *)(SHARED_ADDRESS);
    int i;
    uint32_t *base_memory_pa;

    range = data->memory_range;
    base_memory = data->start_memory;

    for (i = 0; i < range; i++) {
        base_memory_pa = (uint32_t *)(uint32_t)va_to_pa(base_memory, 0);
        *dump_base = *base_memory_pa;
        dump_base++;
        base_memory += 4;
    }
    data->type = MEMORY;
    interrupt_guest_inject(MO_GUEST, MO_VIRQ, 0, INJECT_SW);
    //monitor_notify_guest(MO_GUEST);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t monitor_detect_fault()
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    return ret;
}

hvmm_status_t monitor_recovery_guest()
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    return ret;
}

hvmm_status_t monitor_request(int irq, vmid_t vmid, int address)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    return ret;
}

hvmm_status_t monitor_notify_guest(vmid_t vmid)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    interrupt_guest_inject(vmid, MO_VIRQ, 0, INJECT_SW);

    return ret;
}

hvmm_status_t monitor_init()
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    return ret;
}
