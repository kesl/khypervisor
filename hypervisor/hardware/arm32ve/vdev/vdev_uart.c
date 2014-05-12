
#include <vdev.h>
#include <log/print.h>
#include <log/uart_print.h>
#include <guest.h>

#define VUART_BASE_ADDR 0x12C10000
#define PUART_BASE_ADDR 0x12C20000

#define    readl(a)         (*(volatile unsigned int *)(a))
#define    readb(a)         (*(volatile unsigned char *)(a))
#define    reads(a)         (*(volatile unsigned short *)(a))
#define    writel(v, a)         (*(volatile unsigned int *)(a) = (v))
#define    writeb(v, a)         (*(volatile unsigned char *)(a) = (v))
#define    writes(v, a)         (*(volatile unsigned short *)(a) = (v))

static struct vdev_memory_map _vdev_uart_info = {
   .base = VUART_BASE_ADDR,
   .size = 0x1000,

};


static hvmm_status_t vdev_uart_access_handler(uint32_t write, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size)
{


  unsigned int vmid = guest_current_vmid();

  unsigned char op = rx_status_flag();
  //printH("## char: %d, vmid: %d \n", op, vmid);

  

  if (op == '1' && vmid == 0)
    ;
  else if (op == '2' && vmid == 1)
    ;
  else if ( op == 56 || op == 0 )
    ;
  else if ( op == '3')
    ;
  else
    return HVMM_STATUS_SUCCESS;
   
//  printH("insert\n");
  if (access_size == VDEV_ACCESS_WORD) {
        if(write) {
            uint32_t *pUART = (uint32_t *) PUART_BASE_ADDR;
            uint32_t c = *pvalue;
            writel(c, (pUART + offset/4));
            printh("%s[%d] write addr=%x, %d\n", __func__, __LINE__, pUART + offset, *pvalue);
        } else {
            uint32_t *pUART = (uint32_t *) PUART_BASE_ADDR;
            uint32_t c = readl(pUART + offset/4);
            *pvalue = c;
            printh("%s[%d] read addr=%x, %d\n", __func__, __LINE__, pUART + offset, *pvalue);
        }
    } else if (access_size == VDEV_ACCESS_HWORD) {
        if(write) {
            uint16_t *pUART = (uint16_t *) PUART_BASE_ADDR;
            uint16_t c = (uint16_t) ((*pvalue) & 0xFFFF);
            writes(c, pUART + offset/2);
            printh("%s[%d] write addr=%x, %d\n", __func__, __LINE__, pUART + offset, *pvalue);
        } else {
            uint16_t *pUART = (uint16_t *) PUART_BASE_ADDR;
            uint16_t c = reads(pUART + offset/2) & 0xFFFF;
            *pvalue = (uint32_t)c;
            printh("%s[%d] read addr=%x, %d\n", __func__, __LINE__, pUART + offset, *pvalue);
        }
    } else if (access_size == VDEV_ACCESS_BYTE) {
        if(write) {
            uint8_t *pUART = (uint8_t *) PUART_BASE_ADDR;
            uint8_t c = (uint8_t) ((*pvalue) & 0xFF);
            writeb(c, pUART + offset);
            printh("%s[%d] write addr=%x, %d\n", __func__, __LINE__, pUART + offset, *pvalue);
        } else {
            uint8_t *pUART = (uint8_t *) PUART_BASE_ADDR;
            uint8_t c = readb(pUART + offset) & 0xFF;
            *pvalue = (uint32_t) c;
            printh("%s[%d] read addr=%x, %d\n", __func__, __LINE__, pUART + offset, *pvalue);
        }
    }

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t vdev_uart_read(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_uart_info.base;
  //  printh("vdev read:'%s'\n", __func__);
    return vdev_uart_access_handler(0, offset, info->value, info->sas);
}

static hvmm_status_t vdev_uart_write(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_uart_info.base;
  //  printh("vdev write:'%s'\n", __func__);
    return vdev_uart_access_handler(1, offset, info->value, info->sas);
}

static int32_t vdev_uart_post(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint8_t isize = 4;

    if (regs->cpsr & 0x20) /* Thumb */
        isize = 2;

    regs->pc += isize;

    return 0;
}

static int32_t vdev_uart_check(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_uart_info.base;

    if (info->fipa >= _vdev_uart_info.base &&
        offset < _vdev_uart_info.size)
        return 0;
    return VDEV_NOT_FOUND;
}

static hvmm_status_t vdev_uart_reset(void)
{
    printh("vdev init:'%s'\n", __func__);
    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_uart_ops = {
    .init = vdev_uart_reset,
    .check = vdev_uart_check,
    .read = vdev_uart_read,
    .write = vdev_uart_write,
    .post = vdev_uart_post,
};

struct vdev_module _vdev_uart_module = {
    .name = "K-Hypervisor vDevice Uart Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_uart_ops,
};

hvmm_status_t vdev_uart_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(VDEV_LEVEL_LOW, &_vdev_uart_module);
    if (result == HVMM_STATUS_SUCCESS)
        printh("vdev registered:'%s'\n", _vdev_uart_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_uart_module.name, result);
    }

    return result;
}
vdev_module_low_init(vdev_uart_init);
