#include "virq.h"
#include "hyp_config.h"
#include "print.h"
#include <hvmm_trace.h>
#include <vgic.h>
#include <gic.h>

#define VIRQ_MIN_VALID_PIRQ 16
#define VIRQ_NUM_MAX_PIRQS  1024

#define VALID_PIRQ(pirq) (pirq >= VIRQ_MIN_VALID_PIRQ && pirq < VIRQ_NUM_MAX_PIRQS)

#define VIRQ_MAX_ENTRIES    128

struct virq_entry {
    uint32_t pirq;
    uint32_t virq;
    uint8_t hw;
    uint8_t valid;
};

static struct virq_entry _guest_virqs[NUM_GUESTS_STATIC][VIRQ_MAX_ENTRIES + 1];


hvmm_status_t virq_inject(vmid_t vmid, uint32_t virq, uint32_t pirq, uint8_t hw)
{
    hvmm_status_t result = HVMM_STATUS_BUSY;
    int i;
    struct virq_entry *q = &_guest_virqs[vmid][0];
    
    for( i = 0; i < VIRQ_MAX_ENTRIES; i++ ) {
        if ( q[i].valid == 0 ) {
            q[i].pirq = pirq;
            q[i].virq = virq;
            q[i].hw = hw;
            q[i].valid = 1;
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    }

    printh( "virq: queueing virq %d pirq %d to vmid %d %s\n", virq, pirq, vmid, result == HVMM_STATUS_SUCCESS ? "done" : "failed");
    return result;
}

static void virq_flush(vmid_t vmid)
{
    /* Actual injection of queued VIRQs takes place here */

    int i;
    int count = 0;
    struct virq_entry *entries = &_guest_virqs[vmid][0];

    HVMM_TRACE_ENTER();
    for( i = 0; i < VIRQ_MAX_ENTRIES; i++) {
        if ( entries[i].valid ) {
            hvmm_status_t result;
            if ( entries[i].hw ) {
                result = vgic_inject_virq_hw(entries[i].virq, VIRQ_STATE_PENDING, GIC_INT_PRIORITY_DEFAULT, entries[i].pirq);
            } else {
                result = vgic_inject_virq_sw(entries[i].virq, VIRQ_STATE_PENDING, GIC_INT_PRIORITY_DEFAULT, smp_processor_id(), 1);
            }

            /* Forget */
            entries[i].valid = 0;

            if (result != HVMM_STATUS_SUCCESS ) {
                break;
            }

            count++;
        }
    }
    printh( "virq: injected %d virqs to vmid %d \n", count, vmid );
    HVMM_TRACE_EXIT();
}

hvmm_status_t virq_init(void)
{
    int i, j;

    for( i = 0; i < NUM_GUESTS_STATIC; i++) {
        for( j = 0; j < (VIRQ_MAX_ENTRIES + 1); j++ ) {
            _guest_virqs[i][j].valid = 0;
        }
    }
    vgic_setcallback_virq_flush(&virq_flush);
    return HVMM_STATUS_SUCCESS;
}

