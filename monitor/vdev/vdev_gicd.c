#include "vdev_gicd.h"
#include <context.h>
#include <print.h>
#include <gic_regs.h>
#include <vdev.h>

#define VGICD_ITLINESNUM    128
#define VGICD_TYPER_DEFAULT ((VGICD_ITLINESNUM >> 5) - 1)   // Lines:128, CPU:0, Security Extenstin:No
#define VGICD_IIDR_DEFAULT  (0x43B) // Cortex-A15 */
#define VGICD_NUM_IGROUPR   (VGICD_ITLINESNUM/32)
#define VGICD_NUM_IENABLER  (VGICD_ITLINESNUM/32)

/* Virtual GIC Distributor */
struct gicd_regs{
    uint32_t CTLR;              /*0x000 RW*/
    uint32_t TYPER;             /*      RO*/
    uint32_t IIDR;              /*      RO*/

    uint32_t IGROUPR[VGICD_NUM_IGROUPR];       /* 0x080 */
    uint32_t ISCENABLER[VGICD_NUM_IENABLER];    /* 0x100, ISENABLER/ICENABLER */
    uint32_t ISPENDR[32];       /* 0x200, ISPENDR/ICPENDR */
    uint32_t ISACTIVER[32];     /* 0x300, ISACTIVER/ICACTIVER */
    uint32_t IPRIORITYR[128];   /* 0x400 */
    uint32_t ITARGETSR[128];    /* 0x800 [0]: RO, Otherwise, RW */
    uint32_t ICFGR[64];         /* 0xC00 */

                                /* Cortex-A15 */
                                    /* 0xD00 GICD_PPISR RO */
                                    /* 0xD04 ~ 0xD1C GICD_SPISRn RO */

    uint32_t NSACR[64];         /* 0xE00 */
    uint32_t SGIR;              /* 0xF00 WO */
    uint32_t CPENDSGIR[4];      /* 0xF10 CPENDSGIR 0xF20 SPENDGIR */

                                /* 0xFD0 ~ 0xFFC RO Cortex-A15 PIDRn, CIDRn */
};

struct gicd_handler_entry {
    uint32_t offset;
    vdev_callback_t handler;
};

static hvmm_status_t handler_000(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size);
static hvmm_status_t handler_ISCENABLER(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size);
static hvmm_status_t handler_ISCPENDR(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size);
static hvmm_status_t handler_ISCACTIVER(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size);
static hvmm_status_t handler_IPRIORITYR(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size);
static hvmm_status_t handler_ITARGETSR(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size);
static hvmm_status_t handler_ICFGR(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size);
static hvmm_status_t handler_PPISPISR_CA15(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size);
static hvmm_status_t handler_NSACR(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size);
static hvmm_status_t handler_F00(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size);

static vdev_info_t _vdev_info;
static struct gicd_regs _regs[NUM_GUESTS_STATIC]; 

static struct gicd_handler_entry _handler_map[0x10] = {
/* 0x00 ~ 0x0F */
    { 0x00, handler_000 },              /* CTLR, TYPER, IIDR, IGROUPR */
    { 0x01, handler_ISCENABLER },       /* ISENABLER, ICENABLER */
    { 0x02, handler_ISCPENDR },         /* ISPENDR, ICPENDR */
    { 0x03, handler_ISCACTIVER },       /* ISACTIVER */
    { 0x04, handler_IPRIORITYR },       /* IPRIORITYR */
    { 0x05, handler_IPRIORITYR },  
    { 0x06, handler_IPRIORITYR },  
    { 0x07, handler_IPRIORITYR },
    { 0x08, handler_ITARGETSR },        /* ITARGETSR */
    { 0x09, handler_ITARGETSR },
    { 0x0A, handler_ITARGETSR },
    { 0x0B, handler_ITARGETSR },
    { 0x0C, handler_ICFGR },            /* ICFGR */
    { 0x0D, handler_PPISPISR_CA15 },    /* PPISPISR */
    { 0x0E, handler_NSACR },            /* NSACR */
    { 0x0F, handler_F00 },              /* SGIR, CPENDSGIR, SPENDGIR, ICPIDR2 */
};    

static hvmm_status_t access_handler(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    printh( "%s: %s offset:%d value:%x\n", __FUNCTION__, write ? "write" : "read", offset, write ? *pvalue : (uint32_t) pvalue );

    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    uint8_t offsetidx = (uint8_t) ((offset & 0xF00) >> 8);
    result = _handler_map[offsetidx].handler(write, offset, pvalue, access_size);
    return result;
}



static hvmm_status_t handler_000(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    //CTLR;              /*0x000 RW*/
    //TYPER;             /*      RO*/
    //IIDR;              /*      RO*/
    //IGROUPR[32];       /* 0x080 ~ 0x0FF */

    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vmid_t vmid = context_current_vmid();
    struct gicd_regs *regs = &_regs[vmid];
    uint32_t woffset = offset/4;

    switch(woffset) {
        case GICD_CTLR: /* RW */
            if ( write ) {
                regs->CTLR = *pvalue;
            } else {
                *pvalue = regs->CTLR;
            }
            result = HVMM_STATUS_SUCCESS;
            break;

        case GICD_TYPER:    /* RO */
            if ( write == 0 ) {
                *pvalue = VGICD_TYPER_DEFAULT;
                result = HVMM_STATUS_SUCCESS;
            }
            break;

        case GICD_IIDR:     /* RO */
            if ( write == 0 ) {
                *pvalue = VGICD_IIDR_DEFAULT;
                result = HVMM_STATUS_SUCCESS;
            }
            break;

        default:            /* RW GICD_IGROUPR */
            {
                int igroup = woffset - GICD_IGROUPR;
                if ( igroup >= 0 && igroup < VGICD_NUM_IGROUPR ) {
                    if ( write ) {
                        regs->IGROUPR[igroup] = *pvalue;
                    } else {
                        *pvalue = regs->IGROUPR[igroup];
                    }
                    result = HVMM_STATUS_SUCCESS;
                }
            }
            break;
    }

    if ( result != HVMM_STATUS_SUCCESS ) {
        printh("vgicd: invalid access offset:%x write:%d\n", offset, write );
    }

    return result;
}

static hvmm_status_t handler_ISCENABLER(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh( "vgicd:%s: not implemented\n", __FUNCTION__ );
    vmid_t vmid = context_current_vmid();
    struct gicd_regs *regs = &_regs[vmid];

    offset >>= 2;
    if ( offset < (GICD_ISENABLER + VGICD_NUM_IENABLER) ) {
        /* ISENABLER */
        if ( write ) {
            regs->ISCENABLER[offset - GICD_ISENABLER] |= *pvalue;
        } else {
            *pvalue = regs->ISCENABLER[offset - GICD_ISENABLER];
        }
        result = HVMM_STATUS_SUCCESS;
    } else if ( offset >= GICD_ICENABLER && offset < (GICD_ICENABLER + VGICD_NUM_IENABLER)) {
        /* ICENABLER */
        if ( write ) {
            regs->ISCENABLER[offset - GICD_ICENABLER] &= ~(*pvalue);
        } else {
            *pvalue = regs->ISCENABLER[offset - GICD_ICENABLER];
        }
        result = HVMM_STATUS_SUCCESS;
    }
    return result;
}

static hvmm_status_t handler_ISCPENDR(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh( "vgicd:%s: not implemented\n", __FUNCTION__ );
    return result;
}

static hvmm_status_t handler_ISCACTIVER(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh( "vgicd:%s: not implemented\n", __FUNCTION__ );
    return result;
}

static hvmm_status_t handler_IPRIORITYR(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh( "vgicd:%s: not implemented\n", __FUNCTION__ );
    return result;
}

static hvmm_status_t handler_ITARGETSR(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh( "vgicd:%s: not implemented\n", __FUNCTION__ );
    return result;
}

static hvmm_status_t handler_ICFGR(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh( "vgicd:%s: not implemented\n", __FUNCTION__ );
    return result;
}

static hvmm_status_t handler_PPISPISR_CA15(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh( "vgicd:%s: not implemented\n", __FUNCTION__ );
    return result;
}

static hvmm_status_t handler_NSACR(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh( "vgicd:%s: not implemented\n", __FUNCTION__ );
    return result;
}

static hvmm_status_t handler_F00(uint32_t write, uint32_t offset, uint32_t *pvalue, vdev_access_size_t access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh( "vgicd:%s: not implemented\n", __FUNCTION__ );
    return result;
}

hvmm_status_t vdev_gicd_init(uint32_t base_addr)
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    _vdev_info.name     = "vgicd";
    _vdev_info.base     = base_addr; 
    _vdev_info.size     = 4096;
    _vdev_info.handler  = access_handler;

    result = vdev_reg_device(&_vdev_info);
    if ( result == HVMM_STATUS_SUCCESS ) {
        printh("%s: vdev registered:'%s'\n", __FUNCTION__, _vdev_info.name);
    } else {
        printh("%s: Unable to register vdev:'%s' code=%x\n", __FUNCTION__, _vdev_info.name, result);
    }
    return result;
}
