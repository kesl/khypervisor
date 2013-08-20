#include "vgic.h"
#include "gic.h"
#include "gic_regs.h"
#include "hvmm_trace.h"
#include "armv7_p15.h"

/* Cortex-A15: 25 (PPI6) */
#define VGIC_MAINTENANCE_INTERRUPT_IRQ  25

#define VGIC_MAX_LISTREGISTERS          64
#define VGIC_SIGNATURE_INITIALIZED      0x45108EAD
#define VGIC_READY()                    (_vgic.initialized == VGIC_SIGNATURE_INITIALIZED) 
#define VGIC_SLOT_NOTFOUND              (0xFFFFFFFF)

/* TODO: move to asm-arm_inline.h */
#define asm_clz(x)      ({ uint32_t rval; asm volatile(\
                                " clz %0, %1\n\t" \
                                : "=r" (rval) : "r" (x) :); rval;})

/*
 * Operations:
 * - INIT - [V] Number of List Registers
 * - INIT - [V] route/enable maintenance IRQ
 * - INIT - [V] enable VGIC
 * - INIT - [V] Enable/Disable Virtual IRQ HCR.VI[7]
 *       - vgic_inject_enable()
 * - [V] Inject virq, slot(lr), hw?, state=pending,priority,
 *      - hw:1 - physicalID
 *      - hw:0 - cpuid, EOI(->maintenance int)
 *      GICH_ELSR[VIRQ/32][VIRQ%32] == 1, Free
 *      Otherwise, Used
 *
 *      - vgic_inject_virq( virq, slot, state, priority, hw, physrc, maintenance )
 *      - vgic_inject_virq_hw( virq, state, priority, pirq)
 *      - vgic_inject_virq_sw( virq, state, priority, cpuid, maintenance )
 *
 *  - [*] ISR: Maintenance IRQ
 *      Check VICH_MISR
 *          [V] EOI - At least one VIRQ EOI
 *          [ ] U - Underflow - Non or one valid interrupt in LRs
 *          [ ] LRENP - LI Entry Not Present (no valid interrupt for an EOI request)
 *          [ ] NP - No Pending Interrupt
 *          [ ] VGrp[0/1][E/D] 
 *  - [V] Context Switch:
 *  Saved/Restored Registers:
 *      - GICH_LR
 *      - GICH_APR
 *      - GICH_HCR
 *      - GICH_VMCR
 *  Saved/Restored Data:
 *      - Free Interrupts
 *
 */

struct vgic {
    volatile uint32_t *base;    /* Base address of VGIC (Virtual Interface Control Registers) */
    uint32_t num_lr;            /* Number of List Registers */
    uint32_t initialized;       /* vgic module initialized if == VGIC_SIGNATURE_INITIALIZED */
    uint64_t valid_lr_mask;
};

hvmm_status_t vgic_injection_enable(uint8_t enable);

static struct vgic _vgic;

static uint32_t vgic_find_free_slot(void)
{
    uint32_t slot;
    uint32_t shift = 0;
    HVMM_TRACE_ENTER();
    slot = _vgic.base[GICH_ELSR0];
    if ( slot == 0 && _vgic.num_lr > 32 ) {
        /* first 32 slots are occupied, try the later */
        slot = _vgic.base[GICH_ELSR1];
        shift = 32;
    }

    HVMM_TRACE_HEX32("elsrn:", slot);

    if ( slot ) {
        slot &= -(slot);
        slot = (31 - asm_clz(slot));
        slot += shift;
    } else {
        /* 64 slots are fully occupied */
        slot = VGIC_SLOT_NOTFOUND;
    }
    HVMM_TRACE_EXIT();
    return slot;
}

/*
 * Test if the List Registers 'slot' is free
 * Return
 *  Free - 'slot' is returned
 *  Occupied but another free slot found - new free slot
 *  Fully occupied - VGIC_SLOT_NOTFOUND
 */

static uint32_t vgic_is_free_slot(uint32_t slot)
{
    uint32_t free_slot = VGIC_SLOT_NOTFOUND;
    
    if ( slot < 32 ) {
        if ( _vgic.base[GICH_ELSR0] & (1 << slot) )
            free_slot = slot;
    } else {
        if ( _vgic.base[GICH_ELSR1] & (1 << (slot - 32)) )
            free_slot = slot;
    }

    if ( free_slot != slot ) {
        free_slot = vgic_find_free_slot();
    }

    return free_slot;
}

static void _vgic_dump_status(void)
{
    /*
     * === VGIC Status Summary ===
     * Initialized: Yes
     * Num ListRegs: n
     * Hypervisor Control
     *  - Enabled: Yes
     *  - EOICount: 
     *  - Underflow:
     *  - LRENPIE:
     *  - NPIE:
     *  - VGrp0EIE:
     *  - VGrp0DIE:
     *  - VGrp1EIE:
     *  - VGrp1DIE:
     * VGIC Type
     *  - ListRegs:
     *  - PREbits:
     *  - PRIbits:
     * Virtual Machine Control
     *  - 
     */
    uart_print("=== VGIC Status ===\n\r");
    uart_print(" Initialized:"); uart_print( ( VGIC_READY() ? "Yes" : "No" ) ); uart_print("\n\r");
    uart_print(" Num ListRegs:"); uart_print_hex32( _vgic.num_lr ); uart_print("\n\r");
    uart_print(" LR_MASK:"); uart_print_hex64( _vgic.valid_lr_mask ); uart_print("\n\r");
}

static void _vgic_dump_regs(void)
{
    /*
     * HCR * VTR * VMCR * MISR * EISR0 * EISR1 * ELSR0 * ELSR1 * APR * LR0~n
     */
    int i;
    HVMM_TRACE_ENTER();

    uart_print("  hcr:"); uart_print_hex32( _vgic.base[GICH_HCR] ); uart_print("\n\r");
    uart_print("  vtr:"); uart_print_hex32( _vgic.base[GICH_VTR] ); uart_print("\n\r");
    uart_print(" vmcr:"); uart_print_hex32( _vgic.base[GICH_VMCR] ); uart_print("\n\r");
    uart_print(" misr:"); uart_print_hex32( _vgic.base[GICH_MISR] ); uart_print("\n\r");
    uart_print("eisr0:"); uart_print_hex32( _vgic.base[GICH_EISR0] ); uart_print("\n\r");
    uart_print("eisr1:"); uart_print_hex32( _vgic.base[GICH_EISR1] ); uart_print("\n\r");
    uart_print("elsr0:"); uart_print_hex32( _vgic.base[GICH_ELSR0] ); uart_print("\n\r");
    uart_print("elsr1:"); uart_print_hex32( _vgic.base[GICH_ELSR1] ); uart_print("\n\r");
    uart_print("  apr:"); uart_print_hex32( _vgic.base[GICH_APR] ); uart_print("\n\r");

    uart_print("   LR:\n\r"); 
    for( i = 0; i < _vgic.num_lr; i++ ) {
        if ( vgic_is_free_slot(i) != i ) {
            uart_print_hex32( _vgic.base[GICH_LR + i] ); uart_print(" - "); uart_print_hex32(i); uart_print("\n\r");
        }
    }

    HVMM_TRACE_EXIT();
}


static void _vgic_isr_maintenance_irq(int irq, void *pregs, void *pdata)
{
    HVMM_TRACE_ENTER();

    if ( _vgic.base[GICH_MISR] & GICH_MISR_EOI ) {
        /* clean up invalid entries from List Registers */
        uint32_t eisr = _vgic.base[GICH_EISR0];
        uint32_t slot;

        while(eisr) {
            slot = (31 - asm_clz(eisr));
            eisr &= ~(1 << slot);
            _vgic.base[GICH_LR + slot] = 0;
	        uart_print( " slot:"); uart_print_hex32(slot); uart_print("\n\r");
        }

        eisr = _vgic.base[GICH_EISR1];
        while(eisr) {
            slot = (31 - asm_clz(eisr));
            eisr &= ~(1 << slot);
            _vgic.base[GICH_LR + slot + 32] = 0;
	        uart_print( " slot:"); uart_print_hex32(slot); uart_print("\n\r");
        }
    }

    vgic_injection_enable(0);
    _vgic_dump_regs();

    HVMM_TRACE_EXIT();
}

hvmm_status_t vgic_enable(uint8_t enable)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    HVMM_TRACE_ENTER();

    if ( VGIC_READY() ) {

        if ( enable ) {
            uint32_t hcr = _vgic.base[GICH_HCR];

            hcr |= GICH_HCR_EN;
            // hcr |= GICH_HCR_NPIE | GICH_HCR_LRENPIE;

            _vgic.base[GICH_HCR] = hcr;
        } else {
            _vgic.base[GICH_HCR] &= ~(GICH_HCR_EN);
        }

        result = HVMM_STATUS_SUCCESS;
    } 
    HVMM_TRACE_EXIT();
    return result;
}

hvmm_status_t vgic_injection_enable(uint8_t enable)
{
    uint32_t hcr;
    HVMM_TRACE_ENTER();

    hcr = read_hcr();
    if ( enable ) {
        if ( (hcr & HCR_VI) == 0 ) {
            hcr |= HCR_VI;
            write_hcr(hcr);
        }
    } else {
        if ( hcr & HCR_VI ) {
            hcr &= ~(HCR_VI);
            write_hcr(hcr);
        }
    }

	hcr = read_hcr(); uart_print( " updated hcr:"); uart_print_hex32(hcr); uart_print("\n\r");

    HVMM_TRACE_EXIT();

    return HVMM_STATUS_SUCCESS;
}

/*
 * Params
 * @virq            virtual id (seen to the guest as an IRQ)
 * @slot            index to GICH_LR, slot < _vgic.num_lr
 * @state           INACTIVE, PENDING, ACTIVE, or PENDING_ACTIVE
 * @priority        5bit priority
 * @hw              1 - physical interrupt, 0 - otherwise
 * @physrc          hw:1 - Physical ID, hw:0 - CPUID
 * @maintenance     hw:0, requires EOI asserts Virtual Maintenance Interrupt
 */
hvmm_status_t vgic_inject_virq( 
        uint32_t virq, uint32_t slot, virq_state_t state, uint32_t priority, 
        uint8_t hw, uint32_t physrc, uint8_t maintenance )
{
    uint32_t physicalid;
    uint32_t lr_desc;
    hvmm_status_t result = HVMM_STATUS_BUSY;

    HVMM_TRACE_ENTER();

    physicalid = (hw ? physrc : (maintenance << 9) | (physrc & 0x7)) << GICH_LR_PHYSICALID_SHIFT;
    physicalid &= GICH_LR_PHYSICALID_MASK;


    lr_desc = (GICH_LR_HW_MASK & (hw << GICH_LR_HW_SHIFT) ) |
        /* (GICH_LR_GRP1_MASK & (1 << GICH_LR_GRP1_SHIFT) )| */
        (GICH_LR_STATE_MASK & (state << GICH_LR_STATE_SHIFT) ) |
        (GICH_LR_PRIORITY_MASK & ( (priority >> 3)  << GICH_LR_PRIORITY_SHIFT) ) |
        physicalid |
        (GICH_LR_VIRTUALID_MASK & virq );

    slot = vgic_is_free_slot( slot );

    HVMM_TRACE_HEX32("lr_desc:", lr_desc);
    HVMM_TRACE_HEX32("free slot:", slot);

    if ( slot != VGIC_SLOT_NOTFOUND ) {
        _vgic.base[GICH_LR + slot] = lr_desc;
        vgic_injection_enable(1);
        result = HVMM_STATUS_SUCCESS;
    }
    _vgic_dump_regs();

    HVMM_TRACE_EXIT();
    return result;
}

hvmm_status_t vgic_inject_virq_hw( uint32_t virq, virq_state_t state, uint32_t priority, uint32_t pirq)
{
    hvmm_status_t result = HVMM_STATUS_BUSY;
    uint32_t slot;
    HVMM_TRACE_ENTER();

    slot = vgic_find_free_slot();
    HVMM_TRACE_HEX32("slot:", slot);
    if ( slot != VGIC_SLOT_NOTFOUND ) {
        result = vgic_inject_virq( virq, slot, state, priority, 1, pirq, 0 );
    }

    HVMM_TRACE_EXIT();
    return result;
}

hvmm_status_t vgic_inject_virq_sw( uint32_t virq, virq_state_t state, uint32_t priority, uint32_t cpuid, uint8_t maintenance)
{
    hvmm_status_t result = HVMM_STATUS_BUSY;
    uint32_t slot;
    HVMM_TRACE_ENTER();

    slot = vgic_find_free_slot();
    HVMM_TRACE_HEX32("slot:", slot);
    if ( slot != VGIC_SLOT_NOTFOUND ) {
        result = vgic_inject_virq( virq, slot, state, priority, 0, cpuid, maintenance );
    }

    HVMM_TRACE_EXIT();
    return result;
}


hvmm_status_t vgic_maintenance_irq_enable(uint8_t enable)
{
    uint32_t irq = VGIC_MAINTENANCE_INTERRUPT_IRQ;

    HVMM_TRACE_ENTER();
    if ( enable ) {
        gic_test_set_irq_handler( irq, &_vgic_isr_maintenance_irq, 0 );
        gic_test_configure_irq( irq,
                GIC_INT_POLARITY_LEVEL,
                gic_cpumask_current(),
                GIC_INT_PRIORITY_DEFAULT );
    } else {
        gic_test_set_irq_handler( irq, 0, 0 );
        gic_disable_irq( irq );
    }
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}

static uint64_t _vgic_valid_lr_mask( uint32_t num_lr )
{
    uint64_t mask_valid_lr = 0xFFFFFFFFFFFFFFFFULL;
    if ( num_lr < VGIC_MAX_LISTREGISTERS ) {
        mask_valid_lr >>= num_lr;
        mask_valid_lr <<= num_lr;
        mask_valid_lr = ~mask_valid_lr;
    }

    return mask_valid_lr;
}

hvmm_status_t vgic_init(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;

    HVMM_TRACE_ENTER();

    _vgic.base = gic_vgic_baseaddr();
    _vgic.num_lr = (_vgic.base[GICH_VTR] & GICH_VTR_LISTREGS_MASK) + 1;
    _vgic.valid_lr_mask = _vgic_valid_lr_mask( _vgic.num_lr );
    _vgic.initialized = VGIC_SIGNATURE_INITIALIZED;

    vgic_maintenance_irq_enable(1);

    result = HVMM_STATUS_SUCCESS;

    _vgic_dump_status();
    _vgic_dump_regs();

    HVMM_TRACE_EXIT();
    return result;
}

hvmm_status_t vgic_save_status( struct vgic_status *status )
{
    hvmm_status_t result = HVMM_STATUS_SUCCESS;
    int i;

    HVMM_TRACE_ENTER();
    _vgic_dump_regs();

    for( i = 0; i < _vgic.num_lr; i++ ) {
        status->lr[i] = _vgic.base[GICH_LR + i];
    }
    status->hcr = _vgic.base[GICH_HCR];
    status->apr = _vgic.base[GICH_APR];
    status->vmcr = _vgic.base[GICH_VMCR];
    status->saved_once = VGIC_SIGNATURE_INITIALIZED;

    vgic_enable(0);
    HVMM_TRACE_EXIT();
    return result;
}

hvmm_status_t vgic_restore_status( struct vgic_status *status )
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    int i;
    HVMM_TRACE_ENTER();
    if ( status->saved_once == VGIC_SIGNATURE_INITIALIZED ) {
        /* TODO: restore vgic configurations from 'status' */
        for( i = 0; i < _vgic.num_lr; i++) {
            _vgic.base[GICH_LR + i] = status->lr[i];
        }
        _vgic.base[GICH_APR] = status->apr;
        _vgic.base[GICH_VMCR] = status->vmcr;
        _vgic.base[GICH_HCR] = status->hcr;

        vgic_enable(1);
        _vgic_dump_regs();
        result = HVMM_STATUS_SUCCESS;
    }
    HVMM_TRACE_EXIT();
    
    return result;
}

