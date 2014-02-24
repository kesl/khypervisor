#include <slotpirq.h>
#include <k-hypervisor-config.h>
#include <vgic.h>
/* SLOT:PIRQ mapping */
#include <log/print.h>

static uint32_t _guest_pirqatslot[NUM_GUESTS_STATIC][VGIC_NUM_MAX_SLOTS];
static uint32_t _guest_virqatslot[NUM_GUESTS_STATIC][VGIC_NUM_MAX_SLOTS];

void slotpirq_init(void)
{
    int i, j;
    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        for (j = 0; j < VGIC_NUM_MAX_SLOTS; j++) {
            _guest_pirqatslot[i][j] = PIRQ_INVALID;
            _guest_virqatslot[i][j] = VIRQ_INVALID;
        }
    }
}

void slotpirq_set(vmid_t vmid, uint32_t slot, uint32_t pirq)
{
    if (vmid < NUM_GUESTS_STATIC) {
        printh("vgic: setting vmid:%d slot:%d pirq:%d\n", vmid, slot, pirq);
        _guest_pirqatslot[vmid][slot] = pirq;
    }
}

uint32_t slotpirq_get(vmid_t vmid, uint32_t slot)
{
    uint32_t pirq = PIRQ_INVALID;
    if (vmid < NUM_GUESTS_STATIC) {
        pirq = _guest_pirqatslot[vmid][slot];
        printh("vgic: reading vmid:%d slot:%d pirq:%d\n", vmid, slot, pirq);
    }
    return pirq;
}

void slotpirq_clear(vmid_t vmid, uint32_t slot)
{
    slotpirq_set(vmid, slot, PIRQ_INVALID);
}

void slotvirq_set(vmid_t vmid, uint32_t slot, uint32_t virq)
{
    if (vmid < NUM_GUESTS_STATIC) {
        printh("vgic: setting vmid:%d slot:%d virq:%d\n", vmid, slot, virq);
        _guest_virqatslot[vmid][slot] = virq;
    } else {
        printh("vgic: not setting invalid vmid:%d slot:%d virq:%d\n", vmid, slot, virq);
    }
}

uint32_t slotvirq_getslot(vmid_t vmid, uint32_t virq)
{
    uint32_t slot = SLOT_INVALID;
    int i;
    if (vmid < NUM_GUESTS_STATIC) {
        for (i = 0; i < VGIC_NUM_MAX_SLOTS; i++) {
            if (_guest_virqatslot[vmid][i] == virq) {
                slot = i;
                printh("vgic: reading vmid:%d slot:%d virq:%d\n", vmid, slot, virq);
                break;
            }
        }
    }
    return slot;
}

void slotvirq_clear(vmid_t vmid, uint32_t slot)
{
    slotvirq_set(vmid, slot, VIRQ_INVALID);
}


