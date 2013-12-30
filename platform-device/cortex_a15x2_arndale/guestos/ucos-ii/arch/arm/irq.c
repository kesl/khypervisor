#include <stdio.h>
#include <ucos_ii.h>
#include <errno.h>

#include <asm-arm/irq.h>
#include <asm-arm/irqs.h>
#include <asm-arm/gic_regs.h>
#include <asm-arm/ptrace.h>
#include <asm-arm/armv7_p15.h>
#include <asm-arm/board.h>

#define MAX_IRQ_NUM 64

#define GIC_INT_PRIORITY_DEFAULT        0xa0


#define CBAR_PERIPHBASE_MSB_MASK    0x000000FF

#define ARM_CPUID_CORTEXA15   0x412fc0f1

#define MIDR_MASK_PPN        (0x0FFF <<4)
#define MIDR_PPN_CORTEXA15    (0xC0F << 4)


#define GIC_INT_PRIORITY_DEFAULT_WORD    ( (GIC_INT_PRIORITY_DEFAULT << 24 ) \
                                         |(GIC_INT_PRIORITY_DEFAULT << 16 ) \
                                         |(GIC_INT_PRIORITY_DEFAULT << 8 ) \
                                         |(GIC_INT_PRIORITY_DEFAULT ) )
#define GIC_NUM_MAX_IRQS    1024

#define GIC_SIGNATURE_INITIALIZED   0x5108EAD7

/*
 * Note:
 * The proper implementation of GIC in a guest is to access GIC Distributor
 * and GIC CPU Interface as if it were accessing the physical GIC registers
 * and the Hypervisor traps access to GIC Distributor from guests and remap
 * GIC CPU Interface register address to GIC Virtual CPU Interface.
 *
 * However, in the current implementation below, we simply access
 * GIC Virtual CPU Interface directly
 * - simon
 */

/* Determined by Hypervisor's Stage2 Address Translation Table */
#define GIC_BASEADDR_GUEST                (0x10480000)

#define read(address)		*((volatile unsigned int *) (address))
#define write(val, address)	*((volatile unsigned int *) (address)) = val

static OS_MEM *irq_mem;
static INT8U irq_buf[MAX_IRQ_NUM][24];

/*
 * Controller mappings for all interrupt sources:
 */
struct irq_desc irq_desc_table[MAX_IRQ_NUM] =
{ 
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
    {NULL, 0, 0}, {NULL, 0, 0},
};


struct gic {
    unsigned int baseaddr;
    volatile unsigned int *ba_gicd;
    volatile unsigned int *ba_gicc;
    unsigned int lines;
    unsigned int initialized;
};

static struct gic _gic;

static int gic_init_baseaddr(unsigned int *va_base)
{
    unsigned int midr;
    int result = -1;

    midr = read_midr();
    /*
     * Note:
     * We currently support GICv2 with Cortex-A15 only.
     * Other architectures with GICv2 support will be further listed and added for support later
     */
    if ( (midr & MIDR_MASK_PPN) == MIDR_PPN_CORTEXA15) {
        _gic.baseaddr = (unsigned int) va_base;
        _gic.ba_gicd = (unsigned int *) (_gic.baseaddr + GIC_OFFSET_GICD);
        _gic.ba_gicc = (unsigned int *) (_gic.baseaddr + GIC_OFFSET_GICC);

        result = 0;
    } else {
        printf( "GICv2 Unsupported\n\r" );
        result = -1;
    }

    return result;
}

#define VDEV_TIMER_BASE    0x3FFFE000
volatile uint32_t *base = (uint32_t *) VDEV_TIMER_BASE;

int gic_init(void)
{
    int result = -1;

    printf("@%s\n", __func__);

    result = gic_init_baseaddr((unsigned int *) GIC_BASEADDR_GUEST);

    /* enable group0 and group1 interrupts */
    _gic.ba_gicc[GICC_CTLR] |= 0x213;

    /* no priority masking */
    _gic.ba_gicc[GICC_PMR] = 0xFF;

    _gic.lines = 1022;

    result = 0;

    if (result == 0) {
        _gic.initialized = GIC_SIGNATURE_INITIALIZED;
    }

    *base = 0;    

    return result;
}

int setup_irq(unsigned int irq, struct irqaction * new)
{
    struct irqaction *old, **p;
    struct irq_desc *desc = irq_desc_table + irq;

    if (irq >= MAX_IRQ_NUM)
        return -EINVAL;


    p = &desc->action;

    if ((old = *p) != NULL) {
        do {
            p = &old->next;
            old = *p;
        } while (old);
    }

    new->irq = irq;
    *p = new;

    return 0;
}

int request_irq(unsigned int irq, void (*handler)(void), 
		unsigned long irqflags, const char *devname, void *dev_id)
{
    INT8U err;

    int retval;
    struct irqaction *action;

    /* aciton allocation */
    action = (struct irqaction *) OSMemGet(irq_mem, &err);
    if (err == OS_MEM_NO_FREE_BLKS) {
        return -ENOMEM;
    } else if (err == OS_MEM_INVALID_PMEM) {
        return -ENOMEM;
    }

     
    action->handler = handler;
    action->flags =  irqflags;
    action->name = devname;
    action->next = NULL;
    action->dev_id = dev_id;
    retval = setup_irq(irq, action);
    if (retval) {
        err = OSMemPut(irq_mem, (void *)action);
        if (err != OS_NO_ERR) 
            printf(" request_irq OSMemPut error\n");
    }

    return retval;
}

void free_irq(unsigned int irq, void *dev_id)
{
    INT8U err;

    struct irq_desc *desc;
    struct irqaction **p;

    if (irq >= MAX_IRQ_NUM)
        return;

    desc = irq_desc_table + irq;
    p = &desc->action;

    for (;;) {
        struct irqaction *action = *p;

        if (action) {
            struct irqaction **pp = p;

            p = &action->next;
            if (action->dev_id != dev_id)
                continue;

        *pp = action->next;

        err = OSMemPut(irq_mem, (void *)action);
        if (err != OS_NO_ERR) 
            printf(" free_irq OSMemPut error\n");
            return;
        }
        printf("Trying to free already-free IRQ %d\n", irq);
        return;
    }
}

int handle_IRQ_event(struct irqaction * action)
{
    int status = 0;

    do {
        status |= action->flags;
        action->handler();
        action = action->next;
    } while (action);

    return status;
}

void do_IRQ(struct pt_regs *regs)
{
    unsigned int iar;
    int irq;
    struct irq_desc *desc;
    struct irqaction * action;

    iar = _gic.ba_gicc[GICC_IAR];
    irq = iar & GICC_IAR_INTID_MASK;

    if ( irq < _gic.lines ) {
        desc = irq_desc_table + irq;
        action = NULL;
        action = desc->action;

        if (!action) {
            _gic.ba_gicc[GICC_EOIR] = irq;
            _gic.ba_gicc[GICC_DIR] = irq;
            return;
        }

        handle_IRQ_event(action);

        _gic.ba_gicc[GICC_EOIR] = irq;
        _gic.ba_gicc[GICC_DIR] = irq;
    }
}

void init_IRQ()
{
    INT8U err;

    irq_mem = OSMemCreate(&irq_buf[0][0], MAX_IRQ_NUM, 24, &err);
    if (err != OS_NO_ERR)
        printf("OSMemCreate ERR\n");

    gic_init();
}

