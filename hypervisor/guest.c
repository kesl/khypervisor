#include <k-hypervisor-config.h>
#include <guest.h>
#include <timer.h>
#include <interrupt.h>
#include <memory.h>
#include <vdev.h>
#include <log/print.h>
#include <hvmm_trace.h>
#include <smp.h>

#define NUM_GUEST_CONTEXTS        NUM_GUESTS_CPU0_STATIC

#define _valid_vmid(vmid) \
    (guest_first_vmid() <= vmid && guest_last_vmid() >= vmid)

static struct guest_struct guests[NUM_CPUS][NUM_GUEST_CONTEXTS];
static int _current_guest_vmid[NUM_CPUS] = {VMID_INVALID, VMID_INVALID};
static int _next_guest_vmid[NUM_CPUS] = {VMID_INVALID, };
struct guest_struct *_current_guest[NUM_CPUS];
/* further switch request will be ignored if set */
static uint8_t _switch_locked;

static hvmm_status_t guest_save(struct guest_struct *guest,
                        struct arch_regs *regs)
{
    /* guest_hw_save : save the current guest's context*/
    if (_guest_module.ops->save)
        return  _guest_module.ops->save(guest, regs);

    return HVMM_STATUS_UNKNOWN_ERROR;
}

static hvmm_status_t guest_restore(struct guest_struct *guest,
                        struct arch_regs *regs)
{
    /* guest_hw_restore : The next becomes the current */
    if (_guest_module.ops->restore)
        return  _guest_module.ops->restore(guest, regs);

     return HVMM_STATUS_UNKNOWN_ERROR;
}

static hvmm_status_t perform_switch(struct arch_regs *regs, vmid_t next_vmid)
{
    /* _curreng_guest_vmid -> next_vmid */
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    struct guest_struct *guest = 0;
	uint32_t cpu = smp_processor_id();

#if _SMP_
    /* below code is hard code, we will remove this in the future */
    if (cpu) {
        guest_save(&guests[cpu], regs);
        memory_save();
        _current_guest_vmid[cpu] = next_vmid;
        memory_restore(_current_guest_vmid[cpu]);
        guest_restore(&guests[cpu], regs);
        return result;
    }
#endif

    if (_current_guest_vmid[cpu] == next_vmid)
        return HVMM_STATUS_IGNORED; /* the same guest? */

    guest_save(&guests[cpu][_current_guest_vmid[cpu]], regs);
    memory_save();
    interrupt_save(_current_guest_vmid[cpu]);
    vdev_save(_current_guest_vmid[cpu]);

    /* The context of the next guest */
    guest = &guests[cpu][next_vmid];
    _current_guest[cpu] = guest;
    _current_guest_vmid[cpu] = next_vmid;

    /* guest_hw_dump */
    if (_guest_module.ops->dump)
        _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_3, &guest->regs);

    vdev_restore(_current_guest_vmid[cpu]);
    interrupt_restore(_current_guest_vmid[cpu]);
    memory_restore(_current_guest_vmid[cpu]);
    guest_restore(guest, regs);

    return result;
}

hvmm_status_t guest_perform_switch(struct arch_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t cpu = smp_processor_id();

#if _SMP_
    if (cpu) {
        if (_current_guest_vmid[cpu] == VMID_INVALID)
            result = perform_switch(0, 2);
        goto out;
    }

#endif
    if (_current_guest_vmid[cpu] == VMID_INVALID) {
        /*
         * If the scheduler is not already running, launch default
         * first guest. It occur in initial time.
         */
        printh("context: launching the first guest\n");
        result = perform_switch(0, _next_guest_vmid[cpu]);
        /* DOES NOT COME BACK HERE */
    } else if (_next_guest_vmid[cpu] != VMID_INVALID &&
                _current_guest_vmid[cpu] != _next_guest_vmid[cpu]) {
        printh("curr: %x\n", _current_guest_vmid[cpu]);
        printh("next: %x\n", _next_guest_vmid[cpu]);
        /* Only if not from Hyp */
        result = perform_switch(regs, _next_guest_vmid[cpu]);
        _next_guest_vmid[cpu] = VMID_INVALID;
    } else {
        /*
         * Staying at the currently active guest.
         * Flush out queued virqs since we didn't have a chance
         * to switch the context, where virq flush takes place,
         * this time
         */
        vgic_flush_virqs(_current_guest_vmid[cpu]);
    }

#if _SMP_
out:
#endif
    _switch_locked = 0;
    return result;
}

/* Switch to the first guest */
void guest_sched_start(void)
{
    struct guest_struct *guest = 0;
	uint32_t cpu = smp_processor_id();

    printh("[hyp] switch_to_initial_guest:\n");
    /* Select the first guest context to switch to. */
    _current_guest_vmid[cpu] = VMID_INVALID;
    guest = &guests[cpu][0];
    /* guest_hw_dump */
    if (_guest_module.ops->dump)
        _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_0, &guest->regs);
    /* Context Switch with current context == none */
#if _SMP_
    if (cpu) {
        guest_perform_switch(&guest->regs);
    } else {
        guest_switchto(0, 0);
        guest_perform_switch(&guest->regs);
    }
#else
    guest_switchto(0, 0);
    guest_perform_switch(&guest->regs);
#endif
}

vmid_t guest_first_vmid(void)
{
    /* FIXME:Hardcoded for now */
    return 0;
}

vmid_t guest_last_vmid(void)
{
    /* FIXME:Hardcoded for now */
    return 1;
}

vmid_t guest_next_vmid(vmid_t ofvmid)
{
    vmid_t next = VMID_INVALID;

    if (ofvmid == VMID_INVALID)
        next = guest_first_vmid();
    else if (ofvmid < guest_last_vmid()) {
        /* FIXME:Hardcoded */
        next = ofvmid + 1;
    }
    return next;
}

vmid_t guest_current_vmid(void)
{
	uint32_t cpu = smp_processor_id();
    return _current_guest_vmid[cpu];
}

vmid_t guest_waiting_vmid(void)
{
	uint32_t cpu = smp_processor_id();
    return _next_guest_vmid[cpu];
}

void guest_dump_regs(struct arch_regs *regs)
{
    /* guest_hw_dump */
    _guest_module.ops->dump(GUEST_VERBOSE_ALL, regs);
}

hvmm_status_t guest_switchto(vmid_t vmid, uint8_t locked)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t cpu = smp_processor_id();

    /* valid and not current vmid, switch */
    if (_switch_locked == 0) {
        if (!_valid_vmid(vmid))
            result = HVMM_STATUS_BAD_ACCESS;
        else {
            _next_guest_vmid[cpu] = vmid;
            result = HVMM_STATUS_SUCCESS;
            printh("switching to vmid: %x\n", (uint32_t)vmid);
        }
    } else
        printh("context: next vmid locked to %d\n", _next_guest_vmid[cpu]);

    if (locked)
        _switch_locked = locked;

    return result;
}

vmid_t sched_policy_determ_next(void)
{
    vmid_t next = guest_next_vmid(guest_current_vmid());

    if (next == VMID_INVALID)
        next = guest_first_vmid();
    return next;
}

void guest_schedule(void *pdata)
{
    struct arch_regs *regs = pdata;

    /* guest_hw_dump */
    if (_guest_module.ops->dump)
        _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_3, regs);
    /*
     * Note: As of guest_switchto() and guest_perform_switch()
     * are available, no need to test if trapped from Hyp mode.
     * guest_perform_switch() takes care of it
     */

    /* Switch request, actually performed at trap exit */
    guest_switchto(sched_policy_determ_next(), 0);
}

hvmm_status_t guest_init()
{
    struct timer_val timer;
    hvmm_status_t result = HVMM_STATUS_SUCCESS;
    struct guest_struct *guest;
    struct arch_regs *regs = 0;
    int i, j;
    int guest_count;
    int vmid_mul = 0;
    uint32_t cpu = smp_processor_id();

    printh("[hyp] init_guests: enter\n");
    /* Initializes 2 guests */
    for (j = 0; j < NUM_CPUS; j++) {
        guest_count = num_of_guest(j);
        for (i = 0; i < guest_count; i++) {
            /* Guest i @guest_bin_start */
            guest = &guests[cpu][i];
            regs = &guest->regs;
            guest->vmid = i + j * vmid_mul;
            /* guest_hw_init */
            if (_guest_module.ops->init)
                _guest_module.ops->init(guest, regs);
        }
        vmid_mul += guest_count;
    }
    printh("[hyp] init_guests: return\n");
#if _SMP_
    if (cpu)
        return result;
#endif

    /* 100Mhz -> 1 count == 10ns at RTSM_VE_CA15, fast model*/
    timer.interval_us = GUEST_SCHED_TICK;
    timer.callback = &guest_schedule;
    result = timer_set(&timer);
    if (result != HVMM_STATUS_SUCCESS)
        printh("[%s] timer startup failed...\n", __func__);

    return result;
}
