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
#include <timer.h>
#define HVC_TRAP 0xe14fff7c

uint32_t trapped_va, trapped_pa;
void monitor_hvc_pre_handler(vmid_t vmid, struct arch_regs **regs)
{
    uint32_t restore_inst;

    trapped_va = (*regs)->pc - 4;
    trapped_pa = (uint32_t)va_to_pa(vmid, trapped_va, TTBR0);
    restore_inst = monitor_load_inst(vmid, trapped_va);

    if (restore_inst != NOTFOUND)
        writel(restore_inst, trapped_pa);
}

void monitor_hvc_post_handler(vmid_t vmid, struct arch_regs **regs,
                                uint32_t type)
{
    (*regs)->pc -= 4;
    flush_dcache_all();
    invalidate_icache_all();
}

void monitor_hvc_break_handler(vmid_t vmid, struct arch_regs **regs)
{
    monitor_break_guest(vmid);
}

void monitor_hvc_trace_handler(vmid_t vmid, struct arch_regs **regs)
{
    int i;
    uint32_t lr, sp, cur_va;

    struct monitoring_data *data;

    asm volatile(" mrs     %0, lr_svc\n\t" : "=r"(lr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t" : "=r"(sp) : : "memory", "cc");
    data = (struct monitoring_data *)(SHARED_ADDRESS);
    trapped_va = (*regs)->pc - 4;
    cur_va = (*regs)->pc;

    data->type = MONITORING;
    data->caller_va = trapped_va;
    data->callee_va = lr;
    data->inst = 0;
    data->regs.cpsr = (*regs)->cpsr;
    data->regs.pc = (*regs)->pc;
    data->regs.lr = (*regs)->lr;
    data->sp = sp;

    for (i = 0; i < ARCH_REGS_NUM_GPR; i++)
        data->regs.gpr[i] = (*regs)->gpr[i];

    /* Set next trap for retrap */
    /* TODO Needs status of Branch instruction. */
    if (monitor_store_inst(vmid, (*regs)->pc, MONITOR_RETRAP))
        writel(HVC_TRAP, trapped_pa + 4) ;

    monitor_notify_guest(MONITOR_GUEST_VMID);

    /* TODO Detecting fault here. It is not working now.
     * monitor_detect_fault((struct monitor_vmid *)(SHARED_VMID), ori_va, regs);
     */
}

void monitor_hvc_retrap_handler(vmid_t vmid, struct arch_regs **regs)
{
    /* Clean break point at retrap point. It do not need keep break point */
    monitor_clean_inst(vmid, trapped_va, MONITOR_RETRAP);
    /* Set previous pc to trap */
    writel(HVC_TRAP, trapped_pa - 4);
}
static int32_t vdev_hvc_monitor_write(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    vmid_t vmid = guest_current_vmid();

    switch (monitor_inst_type(vmid, regs->pc - 4)) {
    case MONITOR_BREAK_TRAP:
        monitor_hvc_pre_handler(vmid, &regs);
        monitor_hvc_trace_handler(vmid, &regs);
        monitor_hvc_break_handler(vmid, &regs);
        monitor_hvc_post_handler(vmid, &regs, MONITOR_BREAK_TRAP);
        break;
    case MONITOR_TRACE_TRAP:
        monitor_hvc_pre_handler(vmid, &regs);
        monitor_hvc_trace_handler(vmid, &regs);
        monitor_hvc_post_handler(vmid, &regs, MONITOR_TRACE_TRAP);
        break;
    case MONITOR_RETRAP:
        monitor_hvc_pre_handler(vmid, &regs);
        monitor_hvc_retrap_handler(vmid, &regs);
        monitor_hvc_post_handler(vmid, &regs, MONITOR_RETRAP);
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
