#ifndef __VGIC_H__
#define __VGIC_H__
#include "arch_types.h"
#include "hvmm_types.h"


typedef enum {
    VIRQ_STATE_INACTIVE = 0x00,
    VIRQ_STATE_PENDING = 0x01,
    VIRQ_STATE_ACTIVE = 0x02,
    VIRQ_STATE_PENDING_ACTIVE = 0x03,
} virq_state_t;

struct vgic_status {
    uint32_t saved_once;    /* restore only if saved once to avoid dealing with corrupted data */
    uint32_t lr[64];
    uint32_t hcr;
    uint32_t apr;
    uint32_t vmcr;
};

hvmm_status_t vgic_enable(uint8_t enable);
hvmm_status_t vgic_init(void);
hvmm_status_t vgic_init_status( struct vgic_status *status, vmid_t vmid );
hvmm_status_t vgic_save_status( struct vgic_status *status, vmid_t vmid );
hvmm_status_t vgic_restore_status( struct vgic_status *status, vmid_t vmid );
hvmm_status_t vgic_inject_virq_sw( uint32_t virq, virq_state_t state, uint32_t priority, 
                                    uint32_t cpuid, uint8_t maintenance);
hvmm_status_t vgic_inject_virq_hw( uint32_t virq, virq_state_t state, uint32_t priority, uint32_t pirq);
hvmm_status_t vgic_inject_virq( uint32_t virq, uint32_t slot, virq_state_t state, uint32_t priority, 
                                uint8_t hw, uint32_t physrc, uint8_t maintenance );
hvmm_status_t vgic_setcallback_virq_flush(void (*callback)(vmid_t vmid));
#endif
