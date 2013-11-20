#include <slotpirq.h>
#include <hyp_config.h>
#include <vgic.h>
#include <print.h>
/* SLOT:PIRQ mapping */

static uint32_t _guest_pirqatslot[NUM_GUESTS_STATIC][VGIC_NUM_MAX_SLOTS];

void slotpirq_init(void)
{
    int i, j;
    for( i = 0; i < NUM_GUESTS_STATIC; i++ ) {
        for( j = 0; j < VGIC_NUM_MAX_SLOTS; j++ ) {
            _guest_pirqatslot[i][j] = PIRQ_INVALID;
        }
    }
}

void slotpirq_set( vmid_t vmid, uint32_t slot, uint32_t pirq )
{
    if ( vmid < NUM_GUESTS_STATIC ) {
        printh( "vgic: setting vmid:%d slot:%d pirq:%d\n", vmid, slot, pirq );
        _guest_pirqatslot[vmid][slot] = pirq;
    }
}

uint32_t slotpirq_get(vmid_t vmid, uint32_t slot)
{
    uint32_t pirq = PIRQ_INVALID;

    if ( vmid < NUM_GUESTS_STATIC ) {
        pirq = _guest_pirqatslot[vmid][slot];
        printh( "vgic: reading vmid:%d slot:%d pirq:%d\n", vmid, slot, pirq );
    }
    return pirq;
}

void slotpirq_clear(vmid_t vmid, uint32_t slot)
{
    slotpirq_set(vmid, slot, PIRQ_INVALID);
}
