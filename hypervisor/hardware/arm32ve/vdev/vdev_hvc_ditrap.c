/*
 * For Dynamic instrumentation trap module
 * Using HVC instruction.
 */
#define DEBUG
#include <vdev.h>
#include <log/print.h>
#include <asm-arm_inline.h>
#include <monitoring.h>
#include <asm_io.h>
#include <guest.h>
#define HVC_TRAP 0xe14fff7c

static int32_t vdev_hvc_ditrap_write(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
//trap handler
//    uint32_t spsr, lr, sp;
    uint32_t ori_pa, ori_va, restore_inst;
/*
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
    ori_va = regs->pc - 4;
    ori_pa = (uint32_t)va_to_pa(ori_va, TTBR0);

    printH("pa is %x\n", ori_pa);
    printH("va is %x\n", ori_va);

    switch(inst_type(ori_va)){
        case BREAK_TRAP:
            /* Both */
            restore_inst = load_inst(ori_va);
            writel(restore_inst, ori_pa);
            printH("BREAK Traped inst. Restore inst is %x\n", *(uint32_t *)(ori_pa));

            /* Set next trap for retrap */
            /* TODO Needs status of Branch instruction. */
            store_inst(regs->pc, (RETRAP) );
            writel(HVC_TRAP, (uint32_t)va_to_pa(regs->pc, TTBR0));
            /* Restore pc */
            regs->pc -= 4;

            set_manually_select_vmid(1);
//            guest_switchto(1, 0);
            
            break;

        case TRAP:
            /* Target address */
            restore_inst = load_inst(ori_va);
            writel(restore_inst, ori_pa);
            printH("Traped inst. Restore inst is %x\n", *(uint32_t *)(ori_pa));

            /* Set next trap for retrap */
            /* TODO Needs status of Branch instruction. */
            store_inst(regs->pc, RETRAP);
            writel(HVC_TRAP, (uint32_t)va_to_pa(regs->pc, TTBR0));
            /* Restore pc */
            regs->pc -= 4;
            break;
        case RETRAP:
            /* For restoration */
            /* Set retrap at previous pc */
            restore_inst = load_inst(ori_va);
            writel(restore_inst, ori_pa);
            printH("For retrap. Restore inst is %x\n", *(uint32_t *)(ori_pa));
            /* Clean break point at retrap point. It do not need keep break point */
            clean_inst(ori_va, RETRAP);
            /* Set previous pc to trap */
            writel(HVC_TRAP, (uint32_t)va_to_pa(regs->pc-8, TTBR0));
            /* Restore pc */
            regs->pc -= 4;
            break;
    }
  /*  
    writel(restore_inst, ori_pa);
//    *(uint32_t *)(ori_pa) = load_inst(ori_va);
    printH("inst is %x\n", *(uint32_t *)(ori_pa));
    regs->pc -= 4;
*/
    return 0;
}

static int32_t vdev_hvc_ditrap_check(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    if ((info->iss & 0xFFFF) == 0xFFFC)
        return 0;

    return VDEV_NOT_FOUND;
}

static hvmm_status_t vdev_hvc_ditrap_reset(void)
{
    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_hvc_ditrap_ops = {
    .init = vdev_hvc_ditrap_reset,
    .check = vdev_hvc_ditrap_check,
    .write = vdev_hvc_ditrap_write,
};

struct vdev_module _vdev_hvc_ditrap_module = {
    .name = "K-Hypervisor vDevice HVC dynamic instrumentation trap Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_hvc_ditrap_ops,
};

hvmm_status_t vdev_hvc_ditrap_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(VDEV_LEVEL_MIDDLE, &_vdev_hvc_ditrap_module);
    if (result == HVMM_STATUS_SUCCESS)
        printh("vdev registered:'%s'\n", _vdev_hvc_ditrap_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_hvc_ditrap_module.name, result);
    }

    return result;
}
vdev_module_middle_init(vdev_hvc_ditrap_init);
