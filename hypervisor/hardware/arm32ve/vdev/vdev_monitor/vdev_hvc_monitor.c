/*
 * For Dynamic instrumentation trap module
 * Using HVC instruction.
 */
#define DEBUG
#include <vdev.h>
#include <log/print.h>
#include <asm-arm_inline.h>
#include <monitor.h>
#include <asm_io.h>
#include <guest.h>
#define HVC_TRAP 0xe14fff7c
/* 80315514 */

void monitor_hvc_pre_handler(struct arch_regs **regs)
{
    uint32_t restore_inst, ori_va, ori_pa;

    ori_va = (*regs)->pc - 4;
    ori_pa = (uint32_t)va_to_pa(ori_va, TTBR0);

    restore_inst = monitor_load_inst(ori_va);
    writel(restore_inst, ori_pa);
}

void monitor_hvc_post_handler(struct arch_regs **regs, uint32_t type)
{
    /* Restore pc */
    if (type != BREAK_TRAP)
        (*regs)->pc -= 4;
}

void monitor_hvc_break_handler(struct arch_regs **regs)
{
    int i;
    uint32_t ori_va, lr, sp;

    struct monitoring_data *data =
        (struct monitoring_data *)(SHARED_ADDRESS);

    asm volatile(" mrs     %0, lr_svc\n\t" : "=r"(lr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t" : "=r"(sp) : : "memory", "cc");
    ori_va = (*regs)->pc - 4;

    data->type = MONITORING;
    data->caller_va = ori_va;
    data->callee_va = lr;
    data->inst = 0;
    data->regs.cpsr = (*regs)->cpsr;
    data->regs.pc = (*regs)->pc;
    data->regs.lr = (*regs)->lr;
    data->sp = sp;

    for (i = 0; i < ARCH_REGS_NUM_GPR; i++)
        data->regs.gpr[i] = (*regs)->gpr[i];

    monitor_notify_guest(MO_GUEST);

    /* Set next trap for retrap */
    /* TODO Needs status of Branch instruction. */
    monitor_store_inst((*regs)->pc, RETRAP);
    writel(HVC_TRAP, (uint32_t)va_to_pa((*regs)->pc, TTBR0));

    /* Restore pc */
     (*regs)->pc -= 4;

    monitor_break_guest();
}

void monitor_hvc_trace_handler(struct arch_regs **regs)
{
    int i;
    uint32_t ori_va, lr, sp;

    struct monitoring_data *data =
        (struct monitoring_data *)(SHARED_ADDRESS);

    asm volatile(" mrs     %0, lr_svc\n\t" : "=r"(lr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t" : "=r"(sp) : : "memory", "cc");
    ori_va = (*regs)->pc - 4;

    data->type = MONITORING;
    data->caller_va = ori_va;
    data->callee_va = lr;
    data->inst = 0;
    data->regs.cpsr = (*regs)->cpsr;
    data->regs.pc = (*regs)->pc;
    data->regs.lr = (*regs)->lr;
    data->sp = sp;

    for (i = 0; i < ARCH_REGS_NUM_GPR; i++)
        data->regs.gpr[i] = (*regs)->gpr[i];

    monitor_notify_guest(MO_GUEST);

    /* Set next trap for retrap */
    /* TODO Needs status of Branch instruction. */
    monitor_store_inst((*regs)->pc, RETRAP);
    writel(HVC_TRAP, (uint32_t)va_to_pa((*regs)->pc, TTBR0));
}

void monitor_hvc_retrap_handler(struct arch_regs **regs)
{
    uint32_t ori_va;

    ori_va = (*regs)->pc - 4;

    /* Clean break point at retrap point. It do not need keep break point */
    monitor_clean_inst(ori_va, RETRAP);
    /* Set previous pc to trap */
    writel(HVC_TRAP, (uint32_t)va_to_pa((*regs)->pc-8, TTBR0));
}

static int32_t vdev_hvc_monitor_write(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
/*
    uint32_t sp, lr, spsr;
    printH("[hyp] : Dump K-Hypervisor's registers\n\r");
    printH(" - banked regs\n");
    asm volatile(" mrs     %0, sp_usr\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_usr\n\t" : "=r"(lr) : : "memory", "cc");
    printH(" - usr: sp:%x lr:%x\n", sp, lr);
    asm volatile(" mrs     %0, spsr_svc\n\t" : "=r"(spsr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_svc\n\t" : "=r"(lr) : : "memory", "cc");
    printH(" - svc: spsr:%x sp:%x lr:%x\n", spsr, sp, lr);
    asm volatile(" mrs     %0, spsr_irq\n\t" : "=r"(spsr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_irq\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_irq\n\t" : "=r"(lr) : : "memory", "cc");
    printH(" - irq: spsr:%x sp:%x lr:%x\n", spsr, sp, lr);
    printH(" - Current guest's vmid is %d\n", guest_current_vmid());
*/
    switch (monitor_inst_type(regs->pc - 4)) {
    case BREAK_TRAP:
        monitor_hvc_pre_handler(&regs);
        monitor_hvc_break_handler(&regs);
        monitor_hvc_post_handler(&regs, BREAK_TRAP);
        break;
    case TRAP:
        monitor_hvc_pre_handler(&regs);
        monitor_hvc_trace_handler(&regs);
        monitor_hvc_post_handler(&regs, TRAP);
        break;
    case RETRAP:
        monitor_hvc_pre_handler(&regs);
        monitor_hvc_retrap_handler(&regs);
        monitor_hvc_post_handler(&regs, RETRAP);
        break;
    }
    return 0;
}

static int32_t vdev_hvc_monitor_check(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    if ((info->iss & 0xFFFF) == 0xFFFC)
        return 0;

    return VDEV_NOT_FOUND;
}

static hvmm_status_t vdev_hvc_monitor_reset(void)
{
    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_hvc_monitor_ops = {
    .init = vdev_hvc_monitor_reset,
    .check = vdev_hvc_monitor_check,
    .write = vdev_hvc_monitor_write,
};

struct vdev_module _vdev_hvc_monitor_module = {
    .name = "K-Hypervisor vDevice HVC dynamic instrumentation trap Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_hvc_monitor_ops,
};

hvmm_status_t vdev_hvc_monitor_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(VDEV_LEVEL_MIDDLE, &_vdev_hvc_monitor_module);
    if (result == HVMM_STATUS_SUCCESS)
        printh("vdev registered:'%s'\n", _vdev_hvc_monitor_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_hvc_monitor_module.name, result);
    }

    return result;
}
vdev_module_middle_init(vdev_hvc_monitor_init);
