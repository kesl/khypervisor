#include <gic.h>
#include <gic_regs.h>
#include <vdev.h>
#include <asm-arm_inline.h>

#define DEBUG
#include <log/print.h>

/* return the bit position of the first bit set from msb
 * for example, firstbit32(0x7F = 111 1111) returns 7
 */
#define firstbit32(word) (31 - asm_clz(word))

/*
 1. High Priority Registers to be implemented first for test with linux
 2. access size support: 8/16/32bit
 3. notifying enable status changed interrupts to outside
 4. API for interrupt enable status change callback registeration
 */

/* Virtual GIC Distributor */
/* Priority of implementation
 - [V] CTLR, TYPER
 - [V] ICFGR
 - [V] ITARGETSR
 - [V] IPRIORITYR
 - [V] ISCENABLER
 -----------------------
 - [ ]
 */

/* hard coding for arndale, fastmodel */
#define TILinesNumber 7
#define NUM_MAX_VIRQS   256

#define NUM_STATUS_WORDS    (NUM_MAX_VIRQS / 32)
#define VGICE_NUM_ISCPENDR (TILinesNumber + 1)
#define VGICE_NUM_ISCENABLER (TILinesNumber + 1)
#define VGICE_NUM_ISCACTIVER (TILinesNumber + 1)
#define VGICE_NUM_IPRIORITYR (8*(TILinesNumber + 1))
#define VGICE_NUM_ITARGETSR (8*(TILinesNumber + 1))
#define VGICE_NUM_ICFGR (2*(TILinesNumber + 1))
#define VGICE_NUM_SPISR (TILinesNumber + 1)
#define VGICE_NUM_CPENDSGIR (TILinesNumber + 1)
#define VGICE_NUM_SPENDSGIR (TILinesNumber + 1)
#define VGICD_NUM_IGROUPR (TILinesNumber + 1)
#define VGICD_NUM_IENABLER (TILinesNumber + 1)

struct gicd_regs {
    uint32_t CTLR; /*0x000 RW*/
    uint32_t TYPER; /*      RO*/
    uint32_t IIDR; /*      RO*/

    uint32_t IGROUPR[VGICD_NUM_IGROUPR]; /* 0x080 */
    uint32_t ISCENABLER[VGICE_NUM_ISCENABLER]; /* 0x100, ISENABLER/ICENABLER */
    uint32_t ISCPENDR[VGICE_NUM_ISCPENDR]; /* 0x200, ISPENDR/ICPENDR */
    uint32_t ISCACTIVER[VGICE_NUM_ISCACTIVER]; /* 0x300, ISACTIVER/ICACTIVER */
    uint32_t IPRIORITYR[VGICE_NUM_IPRIORITYR]; /* 0x400 */
    uint32_t ITARGETSR[VGICE_NUM_ITARGETSR]; /* 0x800 [0]: RO, Otherwise, RW */
    uint32_t ICFGR[VGICE_NUM_ICFGR]; /* 0xC00 */

    /* Cortex-A15 */
    /* 0xD00 GICD_PPISR RO */
    /* 0xD04 ~ 0xD1C GICD_SPISRn RO */

    uint32_t NSACR[64]; /* 0xE00 */
    uint32_t SGIR; /* 0xF00 WO */
    uint32_t CPENDSGIR[VGICE_NUM_CPENDSGIR];
    /* 0xF10 CPENDSGIR 0xF20 SPENDGIR */

/* 0xFD0 ~ 0xFFC RO Cortex-A15 PIDRn, CIDRn */
};

struct gicd_handler_entry {
    uint32_t offset;
    vdev_callback_t handler;
};

static hvmm_status_t
handler_000(uint32_t write, uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size);
static hvmm_status_t
handler_ISCENABLER(uint32_t write, uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size);
static hvmm_status_t
handler_ISCPENDR(uint32_t write, uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size);
static hvmm_status_t
handler_ISCACTIVER(uint32_t write, uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size);
static hvmm_status_t
handler_IPRIORITYR(uint32_t write, uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size);
static hvmm_status_t
handler_ITARGETSR(uint32_t write, uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size);
static hvmm_status_t
handler_ICFGR(uint32_t write, uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size);
static hvmm_status_t
handler_PPISPISR_CA15(uint32_t write, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t
handler_NSACR(uint32_t write, uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size);
static hvmm_status_t
handler_F00(uint32_t write, uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size);

static struct vdev_memory_map _vdev_gicd_info = { .base =
        CFG_GIC_BASE_PA | GIC_OFFSET_GICD, .size = 4096, };
static struct gicd_regs _regs[NUM_GUESTS_STATIC];

static struct gicd_handler_entry _handler_map[0x10] = {
/* 0x00 ~ 0x0F */
{ 0x00, handler_000 }, /* CTLR, TYPER, IIDR, IGROUPR */
{ 0x01, handler_ISCENABLER }, /* ISENABLER, ICENABLER */
{ 0x02, handler_ISCPENDR }, /* ISPENDR, ICPENDR */
{ 0x03, handler_ISCACTIVER }, /* ISACTIVER */
{ 0x04, handler_IPRIORITYR }, /* IPRIORITYR */
{ 0x05, handler_IPRIORITYR }, { 0x06, handler_IPRIORITYR }, { 0x07,
        handler_IPRIORITYR }, { 0x08, handler_ITARGETSR }, /* ITARGETSR */
{ 0x09, handler_ITARGETSR }, { 0x0A, handler_ITARGETSR }, { 0x0B,
        handler_ITARGETSR }, { 0x0C, handler_ICFGR }, /* ICFGR */
{ 0x0D, handler_PPISPISR_CA15 }, /* PPISPISR */
{ 0x0E, handler_NSACR }, /* NSACR */
{ 0x0F, handler_F00 }, /* SGIR, CPENDSGIR, SPENDGIR, ICPIDR2 */
};

/* old status */
static uint32_t old_vgicd_status[NUM_GUESTS_STATIC][NUM_STATUS_WORDS] = { { 0, }, };

static hvmm_status_t handler_000(uint32_t write, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size)
{
    /* CTLR;              0x000 RW*/
    /* TYPER;             RO*/
    /* IIDR;              RO*/
    /* IGROUPR[32];       0x080 ~ 0x0FF */
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vmid_t vmid = guest_current_vmid();
    struct gicd_regs *regs = &_regs[vmid];
    uint32_t woffset = offset / 4;
    switch (woffset) {
    case GICD_CTLR: /* RW */
        if (write)
            regs->CTLR = *pvalue;
        else
            *pvalue = regs->CTLR;
        result = HVMM_STATUS_SUCCESS;
        break;
    case GICD_TYPER: /* RO */
        if (write == 0) {
            *pvalue = regs->TYPER;
            result = HVMM_STATUS_SUCCESS;
        }
        break;
    case GICD_IIDR: /* RO */
        if (write == 0) {
            *pvalue = regs->IIDR;
            result = HVMM_STATUS_SUCCESS;
        }
        break;
    default: { /* RW GICD_IGROUPR */
        int igroup = woffset - GICD_IGROUPR;
        if (igroup >= 0 && igroup < VGICD_NUM_IGROUPR) {
            if (write)
                regs->IGROUPR[igroup] = *pvalue;
            else
                *pvalue = regs->IGROUPR[igroup];

            result = HVMM_STATUS_SUCCESS;
        }
    }
        break;
    }
    if (result != HVMM_STATUS_SUCCESS)
        printH("vgicd: invalid access offset:%x write:%d\n", offset,
                write);

    return result;
}

static void vgicd_changed_istatus(vmid_t vmid, uint32_t istatus,
        uint8_t word_offset)
{
    uint32_t cstatus; /* changed bits only */
    uint32_t minirq;
    int bit;
    /* irq range: 0~31 + word_offset * size_of_istatus_in_bits */
    minirq = word_offset * 32;
    /* find changed bits */
    cstatus = old_vgicd_status[vmid][word_offset] ^ istatus;
    while (cstatus) {
        uint32_t virq;
        uint32_t pirq;
        bit = firstbit32(cstatus);
        virq = minirq + bit;
        pirq = interrupt_virq_to_pirq(vmid, virq);
        if (pirq != PIRQ_INVALID) {
            /* changed bit */
            if (istatus & (1 << bit)) {
                printh("[%s : %d] enabled irq num is %d\n", __func__,
                        __LINE__, bit + minirq);
                interrupt_host_configure(pirq);
                interrupt_guest_enable(vmid, pirq);
            } else {
                printh("[%s : %d] disabled irq num is %d\n", __func__,
                        __LINE__, bit + minirq);
                interrupt_host_disable(pirq);
            }
        } else {
            printh("WARNING: Ignoring virq %d for guest %d has "
                    "no mapped pirq\n", virq, vmid);
        }
        cstatus &= ~(1 << bit);
    }
    old_vgicd_status[vmid][word_offset] = istatus;
}

static hvmm_status_t handler_ISCENABLER(uint32_t write,
        uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vmid_t vmid = guest_current_vmid();
    struct gicd_regs *regs = &_regs[vmid];
    uint32_t *preg_s;
    uint32_t *preg_c;
    if (write && *pvalue == 0) {
        /* Writes 0 -> Has no effect. */
        result = HVMM_STATUS_SUCCESS;
        return result;
    }
    preg_s = &(regs->ISCENABLER[(offset >> 2) - GICD_ISENABLER]);
    preg_c = &(regs->ISCENABLER[(offset >> 2) - GICD_ICENABLER]);
    if (access_size == VDEV_ACCESS_WORD) {
        if ((offset >> 2) < (GICD_ISENABLER + VGICD_NUM_IENABLER)) {
            /* ISENABLER */
            if (write) {
                *preg_s |= *pvalue;
                vgicd_changed_istatus(vmid, *preg_s,
                        (offset >> 2) - GICD_ISENABLER);
            } else
                *pvalue = *preg_s;

            result = HVMM_STATUS_SUCCESS;
        } else if ((offset >> 2) >= GICD_ICENABLER
                && (offset >> 2)
                        < (GICD_ICENABLER + VGICD_NUM_IENABLER)) {
            /* ICENABLER */
            if (write) {
                *preg_c &= ~(*pvalue);
                vgicd_changed_istatus(vmid, *preg_c,
                        (offset >> 2) - GICD_ICENABLER);
            } else
                *pvalue = *preg_c;

            result = HVMM_STATUS_SUCCESS;
        }
    } else if (access_size == VDEV_ACCESS_HWORD) {
        if ((offset >> 2) < (GICD_ISENABLER + VGICD_NUM_IENABLER)) {
            uint16_t *preg_s16 = (uint16_t *) preg_s;
            preg_s16 += (offset & 0x3) >> 1;
            if (write) {
                *preg_s16 |= (uint16_t) (*pvalue & 0xFFFF);
                vgicd_changed_istatus(vmid, *preg_s,
                        (offset >> 2) - GICD_ISENABLER);
            } else
                *pvalue = (uint32_t) *preg_s16;

            result = HVMM_STATUS_SUCCESS;
        } else if ((offset >> 2) >= GICD_ICENABLER
                && (offset >> 2)
                        < (GICD_ICENABLER + VGICD_NUM_IENABLER)) {
            uint16_t *preg_c16 = (uint16_t *) preg_c;
            preg_c16 += (offset & 0x3) >> 1;
            if (write) {
                *preg_c16 &= ~((uint16_t) (*pvalue & 0xFFFF));
                vgicd_changed_istatus(vmid, *preg_c,
                        (offset >> 2) - GICD_ICENABLER);
            } else
                *pvalue = (uint32_t) *preg_c16;

            result = HVMM_STATUS_SUCCESS;
        }
    } else if (access_size == VDEV_ACCESS_BYTE) {
        if ((offset >> 2) < (GICD_ISENABLER + VGICD_NUM_IENABLER)) {
            uint8_t *preg_s8 = (uint8_t *) preg_s;
            preg_s8 += (offset & 0x3);
            if (write) {
                *preg_s8 |= (uint8_t) (*pvalue & 0xFF);
                vgicd_changed_istatus(vmid, *preg_s,
                        (offset >> 2) - GICD_ISENABLER);
            } else
                *pvalue = (uint32_t) *preg_s8;

            result = HVMM_STATUS_SUCCESS;
        } else if ((offset >> 2) >= GICD_ICENABLER
                && (offset >> 2)
                        < (GICD_ICENABLER + VGICD_NUM_IENABLER)) {
            uint8_t *preg_c8 = (uint8_t *) preg_c;
            preg_c8 += (offset & 0x3);
            if (write) {
                *preg_c8 &= ~((uint8_t) (*pvalue & 0xFF));
                vgicd_changed_istatus(vmid, *preg_c,
                        (offset >> 2) - GICD_ICENABLER);
            } else
                *pvalue = (uint32_t) *preg_c8;

            result = HVMM_STATUS_SUCCESS;
        }
    }
    return result;
}

static hvmm_status_t handler_ISCPENDR(uint32_t write, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vmid_t vmid = guest_current_vmid();
    struct gicd_regs *regs = &_regs[vmid];
    uint32_t *preg_s;
    uint32_t *preg_c;
    preg_s = &(regs->ISCPENDR[(offset >> 2) - GICD_ISPENDR]);
    preg_c = &(regs->ISCPENDR[(offset >> 2) - GICD_ICPENDR]);
    offset >>= 2;
    if (access_size == VDEV_ACCESS_WORD) {
        if ((offset >> 2) < (GICD_ISPENDR + VGICD_NUM_IENABLER)) {
            /* ISPEND */
            if (write)
                *preg_s |= *pvalue;
            else
                *pvalue = *preg_s;
            result = HVMM_STATUS_SUCCESS;
        } else if ((offset >> 2) >= GICD_ICPENDR
                && (offset >> 2)
                        < (GICD_ICPENDR + VGICD_NUM_IENABLER)) {
            /* ICPEND */
            if (write)
                *preg_c &= ~(*pvalue);
            else
                *pvalue = *preg_c;
            result = HVMM_STATUS_SUCCESS;
        }
    }
    printh("vgicd:%s: not implemented\n", __func__);
    return result;
}

static hvmm_status_t handler_ISCACTIVER(uint32_t write,
        uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh("vgicd:%s: not implemented\n", __func__);
    return result;
}

static hvmm_status_t handler_IPRIORITYR(uint32_t write,
        uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vmid_t vmid;
    struct gicd_regs *regs;
    uint32_t *preg;
    vmid = guest_current_vmid();
    regs = &_regs[vmid];
    /* FIXME: Support 8/16/32bit access */
    offset >>= 2;
    preg = &(regs->IPRIORITYR[offset - GICD_IPRIORITYR]);
    if (write)
        *preg = *pvalue;
    else
        *pvalue = *preg;

    result = HVMM_STATUS_SUCCESS;
    return result;
}

static hvmm_status_t handler_ITARGETSR(uint32_t write,
        uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vmid_t vmid;
    struct gicd_regs *regs;
    uint32_t *preg;
    vmid = guest_current_vmid();
    regs = &_regs[vmid];
    preg = &(regs->ITARGETSR[(offset >> 2) - GICD_ITARGETSR]);
    if (access_size == VDEV_ACCESS_WORD) {
        offset >>= 2;
        if (write) {
            if (offset > (GICD_ITARGETSR + 7)) {
                /* RO: ITARGETSR0 ~ 7 */
                *preg = *pvalue;
            }
        } else {
            *pvalue = *preg;
        }
    } else if (access_size == VDEV_ACCESS_HWORD) {
        uint16_t *preg16 = (uint16_t *) preg;
        preg16 += (offset & 0x3) >> 1;
        if (write) {
            if ((offset >> 2) > (GICD_ITARGETSR + 7))
                *preg16 = (uint16_t) (*pvalue & 0xFFFF);
        } else
            *pvalue = (uint32_t) *preg16;
    } else if (access_size == VDEV_ACCESS_BYTE) {
        uint8_t *preg8 = (uint8_t *) preg;
        preg8 += (offset & 0x3);
        if (write) {
            if ((offset >> 2) > (GICD_ITARGETSR + 7))
                *preg8 = (uint8_t) (*pvalue & 0xFF);
        } else
            *pvalue = (uint32_t) *preg8;
    }
    result = HVMM_STATUS_SUCCESS;
    return result;
}

static hvmm_status_t handler_ICFGR(uint32_t write, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vmid_t vmid;
    struct gicd_regs *regs;
    uint32_t *preg;
    vmid = guest_current_vmid();
    regs = &_regs[vmid];
    /* FIXME: Support 8/16/32bit access */
    offset >>= 2;
    preg = &(regs->ICFGR[offset - GICD_ICFGR]);
    if (write)
        *preg = *pvalue;
    else
        *pvalue = *preg;
    result = HVMM_STATUS_SUCCESS;
    return result;
}

static hvmm_status_t handler_PPISPISR_CA15(uint32_t write,
        uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh("vgicd:%s: not implemented\n", __func__);
    return result;
}

static hvmm_status_t handler_NSACR(uint32_t write, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh("vgicd:%s: not implemented\n", __func__);
    return result;
}

static hvmm_status_t handler_F00(uint32_t write, uint32_t offset,
        uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printh("vgicd:%s: not implemented\n", __func__);
    return result;
}

static hvmm_status_t vdev_gicd_access_handler(uint32_t write,
        uint32_t offset, uint32_t *pvalue,
        enum vdev_access_size access_size)
{
    printh("%s: %s offset:%x value:%x access_size : %d\n", __func__,
            write ? "write" : "read", offset,
            write ? *pvalue : (uint32_t) pvalue, access_size);
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    uint8_t offsetidx = (uint8_t) ((offset & 0xF00) >> 8);
    printh("offset_idx : %d\n", offsetidx);
    result = _handler_map[offsetidx].handler(write, offset, pvalue,
            access_size);
    return result;
}

static int32_t vdev_gicd_read(struct arch_vdev_trigger_info *info,
        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_gicd_info.base;

    return vdev_gicd_access_handler(0, offset, info->value, info->sas);
}

static int32_t vdev_gicd_write(struct arch_vdev_trigger_info *info,
        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_gicd_info.base;

    return vdev_gicd_access_handler(1, offset, info->value, info->sas);
}

static int32_t vdev_gicd_post(struct arch_vdev_trigger_info *info,
        struct arch_regs *regs) {
    uint8_t isize = 4;

    if (regs->cpsr & 0x20) /* Thumb */
        isize = 2;

    regs->pc += isize;

    return 0;
}

static int32_t vdev_gicd_check(struct arch_vdev_trigger_info *info,
        struct arch_regs *regs)
{
    uint32_t offset = info->fipa - _vdev_gicd_info.base;

    if (info->fipa >= _vdev_gicd_info.base
            && offset < _vdev_gicd_info.size)
        return 0;
    return VDEV_NOT_FOUND;
}

static hvmm_status_t vdev_gicd_reset_values(void)
{
    hvmm_status_t result = HVMM_STATUS_SUCCESS;
    int i;

    printh("vdev init:'%s'\n", __func__);

    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        /*
         * ITARGETS[0~ 7], CPU Targets are set to 0,
         * due to current single-core support design
         */
        int j = 0;
        uint32_t inc_address = 0x00000000;

        _regs[i].CTLR =
                (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                        + GIC_OFFSET_GICD)));
        _regs[i].TYPER =
                (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                        + GIC_OFFSET_GICD + GICD_OFFSET_TYPER)));
        _regs[i].IIDR =
                (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                        + GIC_OFFSET_GICD + 0x8)));
        printH("vdev init:'%s' vmid:%d, gicd TYPER:%x\n", __func__, i,
                _regs[i].TYPER);
        printH("vdev init:'%s' vmid:%d, gicd IIDR:%x\n", __func__, i,
                _regs[i].IIDR);
        for (j = 0; j < VGICD_NUM_IGROUPR; j++) {
            _regs[i].IGROUPR[j] =
                    (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                            + GIC_OFFSET_GICD + inc_address)));
            inc_address += 0x00000004;

        }

        inc_address = 0x00000000;
        for (j = 0; j < VGICE_NUM_ISCENABLER; j++) {
            _regs[i].ISCENABLER[j] =
                    (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                            + GIC_OFFSET_GICD + GICD_OFFSET_ICENABLER
                            + inc_address)));
            old_vgicd_status[i][(inc_address) >> 2] =
                    _regs[i].ISCENABLER[j];
            inc_address += 0x00000004;
        }
        inc_address = 0x00000000;
        for (j = 0; j < VGICE_NUM_ISCPENDR; j++) {
            _regs[i].ISCPENDR[j] =
                    (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                            + GIC_OFFSET_GICD + GICD_OFFSET_ICPENDR
                            + inc_address)));
            inc_address += 0x00000004;
        }
        inc_address = 0x00000000;
        for (j = 0; j < VGICE_NUM_ISCACTIVER; j++) {
            _regs[i].ISCACTIVER[j] =
                    (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                            + GIC_OFFSET_GICD + GICD_OFFSET_ISCACTIVER
                            + inc_address)));

            inc_address += 0x00000004;
        }

        inc_address = 0x00000000;

        for (j = 0; j < VGICE_NUM_IPRIORITYR; j++) {
            _regs[i].IPRIORITYR[j] =
                    (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                            + GIC_OFFSET_GICD + GICD_OFFSET_IPRIORITYR
                            + inc_address)));

            inc_address += 0x00000004;
        }

        inc_address = 0x00000000;
        for (j = 0; j < VGICE_NUM_ITARGETSR; j++) {
            _regs[i].ITARGETSR[j] =
                    (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                            + GIC_OFFSET_GICD + GICD_OFFSET_ITARGETSR
                            + inc_address)));
            inc_address += 0x00000004;
        }

        inc_address = 0x00000000;
        for (j = 0; j < VGICE_NUM_ICFGR; j++) {
            _regs[i].ICFGR[j] =
                    (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                            + GIC_OFFSET_GICD + GICD_OFFSET_ICFGR
                            + inc_address)));
            inc_address += 0x00000004;
        }

        _regs[i].SGIR =
                (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                        + GIC_OFFSET_GICD + 0xF00)));

        inc_address = 0x00000000;
        for (j = 0; j < VGICE_NUM_CPENDSGIR; j++) {
            _regs[i].CPENDSGIR[j] =
                    (uint32_t) (*((volatile unsigned int*) (CFG_GIC_BASE_PA
                            + GIC_OFFSET_GICD + GICD_OFFSET_CPENDGIR
                            + inc_address)));
            inc_address += 0x00000004;
        }

        inc_address = 0x00000000;
    }

    return result;
}

struct vdev_ops _vdev_gicd_ops = { .init = vdev_gicd_reset_values,
        .check = vdev_gicd_check, .read = vdev_gicd_read, .write =
                vdev_gicd_write, .post = vdev_gicd_post, };

struct vdev_module _vdev_gicd_module = { .name =
        "K-Hypervisor vDevice GICD Module", .author = "Kookmin Univ.",
        .ops = &_vdev_gicd_ops, };

hvmm_status_t vdev_gicd_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(VDEV_LEVEL_LOW, &_vdev_gicd_module);
    if (result == HVMM_STATUS_SUCCESS)
        printH("vdev registered:'%s'\n", _vdev_gicd_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n", __func__,
                _vdev_gicd_module.name, result);
    }

    return result;
}
vdev_module_low_init(vdev_gicd_init);
