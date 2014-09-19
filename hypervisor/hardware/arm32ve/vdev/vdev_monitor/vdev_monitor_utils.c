#include <monitor.h>
#include <interrupt.h>
#include <asm_io.h>
#include <guest.h>
#include <log/print.h>
#include <memory.h>
#include <log/string.h>

#define l1_tbl_index_offset 20
#define l2_tbl_index_offset 12
#define l1_tbl_index(va)   (va >> l1_tbl_index_offset)
#define l2_tbl_index(va)   (va >> (l2_tbl_index_offset & 0xFF))

#define l1_descriptor_address(trans_base, l1_table_index) \
        (trans_base | (l1_table_index << 2))

#define l1_descriptor(trans_base, l1_table_index) \
        readl(l1_descriptor_address(trans_base, l1_table_index))

#define l2_descriptor_address(page_table_base_address, l2_table_index) \
        (page_table_base_address | (l2_table_index << 2))

#define l2_descriptor(page_table_base_address, l2_table_index) \
        readl(l2_descriptor_address(page_table_base_address, l2_table_index))

#define SHARED_ADDRESS      (CFG_MEMMAP_GUEST1_OFFSET + 0xEC00000)
#define SHARED_DUMP_ADDRESS (CFG_MEMMAP_GUEST1_OFFSET + 0xEC00100)
/*
 * Guest's VA to PA Monitoring
 */
uint64_t va_to_pa(uint32_t va, uint32_t ttbr_num)
{
    uint32_t linux_guest_ttbr, l1_des, l2_des;
    if (ttbr_num) {
        uint32_t linux_guest_ttbr1 = get_guest(0).context.regs_cop.ttbr1;
        linux_guest_ttbr = linux_guest_ttbr1;
    } else {
        uint32_t linux_guest_ttbr0 = get_guest(0).context.regs_cop.ttbr0;
        linux_guest_ttbr = linux_guest_ttbr0;
    }
    printh("va_to_pa start===== ttbr is %x, va is %x\n", linux_guest_ttbr, va);
    /*
     * If ttbcr.N is not 0, it needs to modify.
     * (0x3FFFF >> ttbcr.n) << (14 + ttbcr.n)
     */
    l1_des = l1_descriptor((linux_guest_ttbr & 0xFFFFC000), l1_tbl_index(va));
    printh("l1_des is %x\n", l1_des);

    switch (l1_des & 0b11) {
    case 0b00:
        /* Fault */
        printH("[%s : %d] l1 page fault\n", __func__, __LINE__);
        return 0;
    case 0b01:
        /* page table */
        l2_des = l2_descriptor((l1_des & 0xFFFFFC00), l2_tbl_index(va));
        switch (l2_des & 0b11) {
        case 0b00:
            /* Fault */
            printh("[%s : %d] l2 page fault\n", __func__, __LINE__);
            return 0;
        case 0b01:
            /* Large page */
            printh("[%s : %d] Large page table\n", __func__, __LINE__);
            return (l2_des & 0xFFFF0000) | (va & 0xFFFF);
        case 0b10:
        case 0b11:
            /* Small page */
            printh("[%s : %d] Small page table\n", __func__, __LINE__);
            return (l2_des & 0xFFFFF000) | (va & 0xFFF);
        }
        break;
    case 0b10:
        /* Supersection */
        if (l1_des & (1 << 18)) {
            printh("[%s : %d] Supersection\n", __func__, __LINE__);
            return ((l1_des & 0x1E0) << 23) | ((l1_des & 0x00F00000) << 4) |
            (l1_des & 0xFF000000) | (va & 0x00FFFFFF);
        } else {
        /* Section */
            printh("[%s : %d] Section\n", __func__, __LINE__);
            return (l1_des & 0xFFF00000) | (va & 0x000FFFFF);
        }
        break;
    case 0b11:
        /* Reserved */
        printH("[%s : %d] reserved\n", __func__, __LINE__);
        return 0;
    default:
        printH("[%s : %d] ERROR\n", __func__, __LINE__);
        return 0;
    }
    return 0;
}

/*
 * Monitoring point Manager : store_inst, load_inst, clean_inst
 */
uint32_t store_inst(uint32_t va, enum breakpoint_type type)
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

enum breakpoint_type inst_type(uint32_t va)
{
    int i;
    for (i = 0; i < NUM_DI; i++)
        if (inst[i][INST_VA] == va)
            return inst[i][INST_TYPE];
    return 0;
}

uint32_t clean_inst(uint32_t va, enum breakpoint_type type)
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

uint32_t load_inst(uint32_t va)
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

#define MO_GUEST 1
#define MO_VIRQ 20
#include <k-hypervisor-config.h>

#define LIST 0
#define MONITORING 1
#define MEMORY 2

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

hvmm_status_t kmo_memory_dump(void)
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

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t kmo_list(void)
{
    int i;
    struct monitoring_data *data;
    for (i = 0; i < NUM_DI; i++) {
        if (inst[i][INST] != EMPTY) {
            data = (struct monitoring_data *)(SHARED_ADDRESS);
            data->type = LIST;
            data->caller_va = inst[i][INST_VA];
            data->inst = inst[i][INST];
            interrupt_guest_inject(MO_GUEST, MO_VIRQ, 0, INJECT_SW);
        }
    }
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t kmo_run(void)
{
    clean_manually_select_vmid();
    guest_switchto(0, 0);
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t kmo_break(uint32_t va, uint32_t type)
{
    store_inst(va, type);
    writel(HVC_TRAP, (uint32_t)va_to_pa(va, TTBR0));
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t kmo_clean(uint32_t va, uint32_t type)
{
    uint32_t inst;
    inst = load_inst(va);
    if (clean_inst(va, type))
        writel(inst, (uint32_t)va_to_pa(va , TTBR0));

    /* Clean point's retrap point */
    inst = load_inst((va) + 4);
    if (clean_inst((va) + 4 , RETRAP))
        writel(inst, (uint32_t)va_to_pa((va) + 4 , TTBR0));
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t kmo_all_clean()
{
    int i;
    for (i = 0; i < NUM_DI; i++) {
        if (inst[i][INST] != EMPTY)
            kmo_clean(inst[i][INST_VA], TRAP | RETRAP | BREAK_TRAP);
    }
    return HVMM_STATUS_SUCCESS;
}

void kmo_break_handler(struct arch_regs **regs, uint32_t type)
{
    uint32_t restore_inst, ori_va, ori_pa, lr, sp;

    asm volatile(" mrs     %0, lr_svc\n\t" : "=r"(lr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t" : "=r"(sp) : : "memory", "cc");
    ori_va = (*regs)->pc - 4;
    ori_pa = (uint32_t)va_to_pa(ori_va, TTBR0);

    restore_inst = load_inst(ori_va);
    writel(restore_inst, ori_pa);

    if (type != RETRAP) {
        struct monitoring_data *data =
            (struct monitoring_data *)(SHARED_ADDRESS);
        data->type = MONITORING;
        data->caller_va = ori_va;
        data->callee_va = lr;
        data->inst = restore_inst;
        data->regs.cpsr = (*regs)->cpsr;
        data->regs.pc = (*regs)->pc;
        data->regs.lr = (*regs)->lr;
        data->sp = sp;
        int i;

        for (i = 0; i < ARCH_REGS_NUM_GPR; i++)
            data->regs.gpr[i] = (*regs)->gpr[i];
        interrupt_guest_inject(MO_GUEST, MO_VIRQ, 0, INJECT_SW);
    }

    switch (type) {
    case TRAP:
    case BREAK_TRAP:
        /* Set next trap for retrap */
        /* TODO Needs status of Branch instruction. */
        store_inst((*regs)->pc, RETRAP);
        writel(HVC_TRAP, (uint32_t)va_to_pa((*regs)->pc, TTBR0));
        break;
    case RETRAP:
        /* Clean break point at retrap point. It do not need keep break point */
        clean_inst(ori_va, RETRAP);
        /* Set previous pc to trap */
        writel(HVC_TRAP, (uint32_t)va_to_pa((*regs)->pc-8, TTBR0));
        break;
    }

    /* Restore pc */
    (*regs)->pc -= 4;

    if (type == BREAK_TRAP) {
        /* Run other guest for stop this guest */
        set_manually_select_vmid(1);
        guest_switchto(1, 0);
    }
}
