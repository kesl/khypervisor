/*
 * For Dynamic instrumentation module
 * Using virtual device.
 */
#include <vdev.h>
#define DEBUG
#include <log/print.h>
#include <monitoring.h>
#include <asm_io.h>
#include <guest.h>
#define MONITOR_BASE_ADDR 0x3FFFD000
#define HVC_TRAP 0xe14fff7c

struct vdev_monitor_regs {
    uint32_t set_point;
    uint32_t clean_point;
};

static struct vdev_memory_map _vdev_monitor_info = {
   .base = MONITOR_BASE_ADDR,
   .size = 0x1000
};

uint32_t store_inst(uint32_t va, enum breakpoint_type type)
{
    int i;
    uint32_t pa;
    for(i = 0; i < NUM_DI; i++){
        if(inst[i][INST_VA] == va){
            inst[i][INST_TYPE] |= type;
            printH("Already set breakpoint\n");
            return 0;
        }
    }
    for(i = 0; i < NUM_DI; i++){
        if(inst[i][INST] == EMPTY){
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
    for(i = 0; i < NUM_DI; i++)
        if(inst[i][INST_VA] == va)
            return inst[i][INST_TYPE];
    return 0;
}

uint32_t clean_inst(uint32_t va, enum breakpoint_type type)
{
    int i;
    for(i = 0; i < NUM_DI; i++){
        if(inst[i][INST_VA] == va){
            inst[i][INST_TYPE] &= ~(type);
            if(inst[i][INST_TYPE] == 0){
                inst[i][INST] = EMPTY;
                inst[i][INST_VA] = EMPTY;
                return 1;
            }
        break;
/*
            if(type == BOTH){
                inst[i][INST] = EMPTY;
                inst[i][INST_VA] = EMPTY;
                inst[i][INST_TYPE] = EMPTY;
                break;
            } else if(inst[i][INST_TYPE] == type){
                inst[i][INST] = EMPTY;
                inst[i][INST_VA] = EMPTY;
                inst[i][INST_TYPE] = EMPTY;
                break;
            } else if(inst[i][INST_TYPE] == BOTH){
                inst[i][INST_TYPE] = (type == TRAP ? RETRAP : TRAP);
                break;
            }
*/
        }
    }
    return 0;
}

uint32_t load_inst(uint32_t va)
{
    int i, ori_inst;
    for(i = 0; i < NUM_DI; i++){
        if(inst[i][1] == va){
            ori_inst = inst[i][INST];
            return ori_inst;
        }
    }
    printh("[%s : %d] corresponded instruction not found\n", __func__, __LINE__);
    return 0;
}

static void print_monitoring_list(void)
{
    int i;
    for(i = 0; i < NUM_DI; i++){
        if(inst[i][INST] != EMPTY){
            printH("Monitoring va is %x, instruction is %x\n", inst[i][INST_VA], inst[i][INST]);
        }
    }
}

//static struct vdev_monitor_regs monitor_regs[NUM_GUESTS_STATIC];

static hvmm_status_t vdev_monitor_access_handler(uint32_t write, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size)
{
    printh("%s: %s offset:%d value:%x\n", __func__,
            write ? "write" : "read", offset,
            write ? *pvalue : (uint32_t) pvalue);
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    uint32_t inst;
//    unsigned int vmid = guest_current_vmid();
    if (!write) {
        /* READ */
        //TODO Read debugging resources
        switch (offset) {
        case 0x8:
            /* print monitoring list */
//            show_symbole(0);
            print_monitoring_list();
            //*pvalue = 0;
            result = HVMM_STATUS_SUCCESS;
            break;
        case 0x10:
            /* go */
            printH("go!\n");
            clean_manually_select_vmid();
//            guest_switchto(0, 0);
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    } else {
        /* WRITE */
        switch (offset) {
        case 0x0:
            // Set monitoring point
            store_inst(*pvalue, TRAP);
            writel(HVC_TRAP, (uint32_t)va_to_pa(*pvalue, TTBR0));
            result = HVMM_STATUS_SUCCESS;
            break;
        case 0x4:
            // Clean monitoring point           
            inst = load_inst(*pvalue);
            if(clean_inst(*pvalue, TRAP)){
                writel(inst, (uint32_t)va_to_pa(*pvalue , TTBR0)); 
            }
           /* 
            if(inst_type(*pvalue) == TRAP || inst_type(*pvalue) == BOTH)
                writel(load_inst(*pvalue), (uint32_t)va_to_pa(*pvalue , TTBR0)); 
            clean_inst(*pvalue, TRAP);
*/
            result = HVMM_STATUS_SUCCESS;
            break;
        case 0x8:
            /* read-only register, ignored, but no error */
            result = HVMM_STATUS_SUCCESS;
            break;
        case 0xC:
            /* break */
            printH("break!\n");
            store_inst(*pvalue, BREAK_TRAP);
            writel(HVC_TRAP, (uint32_t)va_to_pa(*pvalue, TTBR0));
            result = HVMM_STATUS_SUCCESS;
            break;
        case 0x14:
            // Clean breaking point           
            inst = load_inst(*pvalue);
            if(clean_inst(*pvalue, BREAK_TRAP)){
                writel(inst, (uint32_t)va_to_pa(*pvalue , TTBR0)); 
            }
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    }
    return result;
}

static hvmm_status_t vdev_monitor_read(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_monitor_info.base;

    return vdev_monitor_access_handler(0, offset, info->value, info->sas);
}

static hvmm_status_t vdev_monitor_write(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_monitor_info.base;

    return vdev_monitor_access_handler(1, offset, info->value, info->sas);
}

static int32_t vdev_monitor_post(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint8_t isize = 4;

    if (regs->cpsr & 0x20) /* Thumb */
        isize = 2;

    regs->pc += isize;

    return 0;
}

static int32_t vdev_monitor_check(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_monitor_info.base;

    if (info->fipa >= _vdev_monitor_info.base &&
        offset < _vdev_monitor_info.size)
        return 0;
    return VDEV_NOT_FOUND;
}

static hvmm_status_t vdev_monitor_reset(void)
{
    printh("vdev init:'%s'\n", __func__);
    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_monitor_ops = {
    .init = vdev_monitor_reset,
    .check = vdev_monitor_check,
    .read = vdev_monitor_read,
    .write = vdev_monitor_write,
    .post = vdev_monitor_post,
};

struct vdev_module _vdev_monitor_module = {
    .name = "K-Hypervisor vDevice monitoring Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_monitor_ops,
};

hvmm_status_t vdev_monitor_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(VDEV_LEVEL_LOW, &_vdev_monitor_module);
    if (result == HVMM_STATUS_SUCCESS)
        printh("vdev registered:'%s'\n", _vdev_monitor_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_monitor_module.name, result);
    }

    return result;
}
vdev_module_low_init(vdev_monitor_init);
