#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
#include "hvmm_types.h"

/**
 * @breif   Saves a mapping information to find a virq for injection.
 *
 * We do not consider a sharing device that's why we save only one vmid.
 * @todo    (wonseok): need to change a structure when we support
 *                     a sharing device among guests.
 */
struct virqmap_entry {
    vmid_t vmid;    /**< Guest vm id */
    uint32_t virq;  /**< Virtual interrupt nubmer */
    uint32_t pirq;  /**< Pysical interrupt nubmer */
};

#define VIRQMAP_ENTRY_NOTFOUND  0

const struct virqmap_entry *virqmap_for_pirq(uint32_t pirq);
/**
 * @breif       Returns pirq mapped to virq for vm.
 * @param vmid  Guest vm id
 * @param virq  Virtual interrupt number.
 * @return      physical interrupt number.
 */
uint32_t virqmap_pirq(vmid_t vmid, uint32_t virq);

hvmm_status_t virq_inject(vmid_t vmid, uint32_t virq,
        uint32_t pirq, uint8_t hw);
/**
 * @brief   Initializes virq_entry structure and
            Sets callback function about injection of queued VIRQs.
 * @return  Always returns "success".
 */
hvmm_status_t virq_init(void);



typedef void (*interrupt_handler_t)(int irq, void *regs, void *pdata);

struct interrupt_ops {
    /** Initalize interrupt state */
    hvmm_status_t (*init)(void);

    /** Save registers for context switch */
    hvmm_status_t (*enable)(uint32_t);

    /** Restore registers for context switch */
    hvmm_status_t (*disable)(uint32_t);

    /** Restore registers for context switch */
    hvmm_status_t (*configure)(uint32_t);

    /** Restore registers for context switch */
    hvmm_status_t (*end)(uint32_t);

    /** Restore registers for context switch */
    hvmm_status_t (*inject)(vmid_t, uint32_t, uint32_t);

    /** Restore registers for context switch */
    hvmm_status_t (*save)(void);

    /** Restore registers for context switch */
    hvmm_status_t (*restore)(void);

    /** Dump state of the guest */
    hvmm_status_t (*dump)(void);
};

struct interrupt_module {
    /** tag must be initialized to HAL_TAG */
    uint32_t tag;

    /**
     * Version of the module-specific device API. This value is used by
     * the derived-module user to manage different device implementations.
     * The user who uses this module is responsible for checking
     * the module_api_version and device version fields to ensure that
     * the user is capable of communicating with the specific module
     * implementation.
     *
     */
    uint32_t version;

    /** Identifier of module */
    const char *id;

    /** Name of this module */
    const char *name;

    /** Author/owner/implementor of the module */
    const char *author;

    /** Interrupt Operation */
    struct interrupt_ops *host_ops;
    struct interrupt_ops *guest_ops;
};

extern struct interrupt_module _interrupt_module;

/**
 * @brief   Initializes GIC, VGIC status.
 * <pre>
 * Initialization sequence.
 * 1. Routes IRQ/IFQ to Hyp Exception Vector.
 * 2. Initializes GIC Distributor & CPU Interface and enable them.
 * 3. Initializes and enable Virtual Interface Control.
 * 4. Initializes physical irq, virtual irq status table, and
 *    registers injection callback function that is called
 *    when flushs virtual irq.
 * </pre>
 * @return  If all initialization is success, then returns "success"
 *          otherwise returns "unknown error"
 */
hvmm_status_t interrupt_init(struct virqmap_entry *virqmap);
hvmm_status_t interrupt_request(uint32_t irq, interrupt_handler_t handler);
hvmm_status_t interrupt_host_enable(uint32_t irq);
hvmm_status_t interrupt_host_disable(uint32_t irq);
hvmm_status_t interrupt_host_configure(uint32_t irq);
hvmm_status_t interrupt_guest_inject(vmid_t vmid, uint32_t virq, uint32_t pirq);
hvmm_status_t interrupt_guest_enable(uint32_t irq);
hvmm_status_t interrupt_guest_disable(uint32_t irq);
hvmm_status_t interrupt_save(void);
hvmm_status_t interrupt_restore(void);
void interrupt_service_routine(int irq, void *current_regs, void *pdata);

#endif
