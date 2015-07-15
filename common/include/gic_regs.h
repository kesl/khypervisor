#ifndef __GIC_REGS_H__
#define __GIC_REGS_H__

/* Offsets from GIC Base Address */
#define GIC_OFFSET_GICD     0x1000
#define GIC_OFFSET_GICC     0x2000
#define GIC_OFFSET_GICH     0x4000
#define GIC_OFFSET_GICV     0x5000
#define GIC_OFFSET_GICVI    0x6000

/* Distributor */
#define GICD_CTLR   0x000
#define GICD_TYPER  (0x004/4)
#define GICD_IIDR   (0x008/4)
#define GICD_IGROUPR    (0x080/4)
#define GICD_ISENABLER    (0x100/4)
#define GICD_ICENABLER    (0x180/4)
#define GICD_ISPENDR    (0x200/4)
#define GICD_ICPENDR    (0x280/4)
#define GICD_IPRIORITYR    (0x400/4)
#define GICD_ITARGETSR    (0x800/4)
#define GICD_ICFGR    (0xC00/4)

#define GICD_SGIR   (0xF00/4)
#define GICD_CPENDSGIR  (0xF10/4)
#define GICD_SPENDSGIR  (0xF20/4)

/* Distributor offset */
#define GICD_OFFSET_CTLR   0x000
#define GICD_OFFSET_TYPER  0x004
#define GICD_OFFSET_IIDR   0x008
#define GICD_OFFSET_IGROUPR    0x080
#define GICD_OFFSET_ISENABLER    0x100
#define GICD_OFFSET_ICENABLER    0x180
#define GICD_OFFSET_ISPENDR    0x200
#define GICD_OFFSET_ICPENDR    0x280
#define GICD_OFFSET_ISCACTIVER    0x300
#define GICD_OFFSET_IPRIORITYR    0x400
#define GICD_OFFSET_ITARGETSR    0x800
#define GICD_OFFSET_ICFGR    0xC00
#define GICD_OFFSET_CPENDGIR    0xF10

#define GICD_OFFSET_SGIR    0xF00
#define GICD_OFFSET_CPENDSGIR   0xF10
#define GICD_OFFSET_SPENDSGIR   0xF20

/* CPU Interface */
#define GICC_CTLR    (0x0000/4)
#define GICC_PMR    (0x0004/4)
#define GICC_BPR    (0x0008/4)
#define GICC_IAR    (0x000C/4)
#define GICC_EOIR    (0x0010/4)
#define GICC_DIR    (0x1000/4)

/* Virtual Interface Control */
#define GICH_HCR    (0x00/4)
#define GICH_VTR    (0x04/4)
#define GICH_VMCR   (0x08/4)
#define GICH_MISR   (0x10/4)
#define GICH_EISR0  (0x20/4)
#define GICH_EISR1  (0x24/4)
#define GICH_ELSR0  (0x30/4)
#define GICH_ELSR1  (0x34/4)
#define GICH_APR    (0xF0/4)
#define GICH_LR     (0x100/4)    /* LR0 ~ LRn, n:GICH_VTR.ListRegs */

/* Distributor Register Fields */
#define GICD_CTLR_ENABLE    0x1
#define GICD_TYPE_LINES_MASK    0x01f
#define GICD_TYPE_CPUS_MASK    0x0e0
#define GICD_TYPE_CPUS_SHIFT    5

/* Software Generated Interrupt Fields */
#define GICD_SGIR_TARGET_LIST_FILTER_MASK   (0x3<<24)
#define GICD_SGIR_TARGET_LIST   (0x0<<24)
#define GICD_SGIR_TARGET_OTHER  (0x1<<24)
#define GICD_SGIR_TARGET_SELF  (0x2<<24)

#define GICD_SGIR_CPU_TARGET_LIST_OFFSET    16
#define GICD_SGIR_CPU_TARGET_LIST_MASK  \
    (0xFF<<GICD_SGIR_CPU_TARGET_LIST_OFFSET)
#define GICD_SGIR_SGI_INT_ID_MASK   0xF

/* CPU Interface Register Fields */
#define GICC_CTL_ENABLE     0x1
#define GICC_CTL_EOI        (0x1 << 9)
#define GICC_IAR_INTID_MASK    0x03ff

/* Virtual Interface Control */
#define GICH_HCR_EN             0x1
#define GICH_HCR_NPIE           (0x1 << 3)
#define GICH_HCR_LRENPIE        (0x1 << 2)
#define GICH_HCR_UIE            (0x1 << 1)
#define GICH_VTR_PRIBITS_SHIFT  29
#define GICH_VTR_PRIBITS_MASK   (0x7 << GICH_VTR_PRIVITS_SHIFT)
#define GICH_VTR_PREBITS_SHIFT  26
#define GICH_VTR_PREBITS_MASK   (0x7 << GICH_VTR_PREVITS_SHIFT)
#define GICH_VTR_LISTREGS_MASK  0x3f

#define GICH_LR_VIRTUALID_SHIFT     0
#define GICH_LR_VIRTUALID_MASK      (0x3ff << GICH_LR_VIRTUALID_SHIFT)
#define GICH_LR_PHYSICALID_SHIFT    10
#define GICH_LR_PHYSICALID_MASK     (0x3ff << GICH_LR_PHYSICALID_SHIFT)
#define GICH_LR_CPUID_SHIFT         10
#define GICH_LR_CPUID_MASK          (0x7 << GICH_LR_CPUID_SHIFT)
#define GICH_LR_EOI_SHIFT           19
#define GICH_LR_EOI_MASK            (0x1 << GICH_LR_EOI_SHIFT)
#define GICH_LR_EOI                 (0x1 << GICH_LR_EOI_SHIFT)
#define GICH_LR_PRIORITY_SHIFT      23
#define GICH_LR_PRIORITY_MASK       (0x1f << GICH_LR_PRIORITY_SHIFT)
#define GICH_LR_STATE_SHIFT     28
#define GICH_LR_STATE_MASK      (0x3 << GICH_LR_STATE_SHIFT)
#define GICH_LR_STATE_INACTIVE          (0x0 << GICH_LR_STATE_SHIFT)
#define GICH_LR_STATE_PENDING           (0x1 << GICH_LR_STATE_SHIFT)
#define GICH_LR_STATE_ACTIVE            (0x2 << GICH_LR_STATE_SHIFT)
#define GICH_LR_STATE_PENDING_ACTIVE    (0x3 << GICH_LR_STATE_SHIFT)
#define GICH_LR_GRP1_SHIFT      30
#define GICH_LR_GRP1_MASK       (0x1 << GICH_LR_GRP1_SHIFT)
#define GICH_LR_GRP1            (0x1 << GICH_LR_GRP1_SHIFT)
#define GICH_LR_HW_SHIFT      31
#define GICH_LR_HW_MASK       (0x1 << GICH_LR_HW_SHIFT)
#define GICH_LR_HW            (0x1 << GICH_LR_HW_SHIFT)

#define GICH_MISR_EOI           (1)
#define GICH_MISR_U_SHIFT       (1)
#define GICH_MISR_U             (1 << GICH_MISR_U_SHIFT)
#define GICH_MISR_NP_SHIFT      (3)
#define GICH_MISR_NP            (1 << GICH_MISR_NP_SHIFT)
#endif
