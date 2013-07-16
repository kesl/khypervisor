#ifndef __GIC_REGS_H__
#define __GIC_REGS_H__

/* Offsets from GIC Base Address */
#define GIC_OFFSET_GICD		0x1000
#define GIC_OFFSET_GICC		0x2000
#define GIC_OFFSET_GICH		0x4000
#define GIC_OFFSET_GICV		0x5000
#define GIC_OFFSET_GICVI	0x6000

/* Distributor */
#define GICD_CTLR	0x000
#define GICD_TYPER	(0x004/4)
#define GICD_IIDR	(0x008/4)
#define GICD_ISENABLER	(0x100/4)
#define GICD_ICENABLER	(0x180/4)
#define GICD_IPRIORITYR	(0x400/4)
#define GICD_ITARGETSR	(0x800/4) 
#define GICD_ICFGR	(0xC00/4)

/* CPU Interface */
#define GICC_CTLR	(0x0000/4)
#define GICC_PMR	(0x0004/4)
#define GICC_BPR	(0x0008/4)
#define GICC_IAR	(0x000C/4)
#define GICC_EOIR	(0x0010/4)
#define GICC_DIR	(0x1000/4)

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
#define GICD_CTLR_ENABLE	0x1
#define GICD_TYPE_LINES_MASK	0x01f
#define GICD_TYPE_CPUS_MASK	0x0e0
#define GICD_TYPE_CPUS_SHIFT	5

/* CPU Interface Register Fields */
#define GICC_CTL_ENABLE 	0x1
#define GICC_CTL_EOI    	(0x1 << 9)
#define GICC_IAR_INTID_MASK	0x03ff

#endif
