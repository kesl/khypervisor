#include <k-hypervisor-config.h>
#include <test/tests.h>
#include <version.h>
#include <log/print.h>
#include <hvmm_trace.h>
#include <guest.h>
#include <timer.h>
#include <mm.h>
#include <interrupt.h>
#include <vdev.h>
#include <virq.h>
#include <vmm.h>

#define NUM_GUEST_CONTEXTS        NUM_GUESTS_STATIC

#define _valid_vmid(vmid) \
    (guest_first_vmid() <= vmid && guest_last_vmid() >= vmid)

static struct guest_struct guests[NUM_GUEST_CONTEXTS];
static int _current_guest_vmid = VMID_INVALID;
static int _next_guest_vmid = VMID_INVALID;
struct guest_struct *_current_guest;

/* further switch request will be ignored if set */
static uint8_t _switch_locked;

static hvmm_status_t perform_switch(struct arch_regs *regs, vmid_t next_vmid)
{
    /* _curreng_guest_vmid -> next_vmid */
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    struct guest_struct *guest = 0;

    if (_current_guest_vmid == next_vmid)
        return HVMM_STATUS_IGNORED; /* the same guest? */

    vmm_lock();
    if (regs != 0) {
        /* save the current guest's context */
        guest = &guests[_current_guest_vmid];
        _guest_module.ops->save(guest, regs);
    }
    /* The context of the next guest */
    guest = &guests[next_vmid];
    _current_guest = guest;
    _current_guest_vmid = next_vmid;
    vmm_unlock(guest);

    _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_3, &guest->regs);

    /* The next becomes the current */
    if (regs == 0) {
        /* init -> hyp mode -> guest */
        /*
         * The actual context switching (Hyp to Normal mode)
         * handled in the asm code
         */
        __mon_switch_to_guest_context(&guest->regs);
    } else {
        /* guest -> hyp -> guest */
        _guest_module.ops->restore(guest, regs);
    }
    result = HVMM_STATUS_SUCCESS;

    return result;
}

hvmm_status_t guest_perform_switch(struct arch_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;

    if (_current_guest_vmid == VMID_INVALID) {
        /*
         * If the scheduler is not already running, launch default
         * first guest. It occur in initial time.
         */
        printh("context: launching the first guest\n");
        result = perform_switch(0, _next_guest_vmid);
        /* DOES NOT COME BACK HERE */
    } else if (_next_guest_vmid != VMID_INVALID &&
                _current_guest_vmid != _next_guest_vmid) {
        printh("curr: %x\n", _current_guest_vmid);
        printh("next: %x\n", _next_guest_vmid);
        /* Only if not from Hyp */
        result = perform_switch(regs, _next_guest_vmid);
        _next_guest_vmid = VMID_INVALID;
    } else {
        /*
         * Staying at the currently active guest.
         * Flush out queued virqs since we didn't have a chance
         * to switch the context, where virq flush takes place,
         * this time
         */
        vgic_flush_virqs(_current_guest_vmid);
    }
    _switch_locked = 0;
    return result;
}

/* Switch to the first guest */
void guest_sched_start(void)
{
    struct guest_struct *guest = 0;

    printh("[hyp] switch_to_initial_guest:\n");
    /* Select the first guest context to switch to. */
    _current_guest_vmid = VMID_INVALID;
    guest = &guests[0];
    _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_0, &guest->regs);
    /* Context Switch with current context == none */
    guest_switchto(0, 0);
    guest_perform_switch(&guest->regs);
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
    return _current_guest_vmid;
}

vmid_t guest_waiting_vmid(void)
{
    return _next_guest_vmid;
}

void guest_dump_regs(struct arch_regs *regs)
{
    _guest_module.ops->dump(GUEST_VERBOSE_ALL, regs);
}

hvmm_status_t guest_switchto(vmid_t vmid, uint8_t locked)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;

    /* valid and not current vmid, switch */
    if (_switch_locked == 0) {
        if (!_valid_vmid(vmid))
            result = HVMM_STATUS_BAD_ACCESS;
        else {
            _next_guest_vmid = vmid;
            result = HVMM_STATUS_SUCCESS;
            printh("switching to vmid: %x\n", (uint32_t)vmid);
        }
    } else
        printh("context: next vmid locked to %d\n", _next_guest_vmid);

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

    _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_3, regs);
    /*
     * Note: As of guest_switchto() and guest_perform_switch()
     * are available, no need to test if trapped from Hyp mode.
     * guest_perform_switch() takes care of it
     */

    /* Switch request, actually performed at trap exit */
    guest_switchto(sched_policy_determ_next(), 0);
}

static void guest_init()
{
    struct guest_struct *guest;
    struct arch_regs *regs = 0;
    int i;

    printh("[hyp] init_guests: enter\n");
    /* Initializes 2 guests */
    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        /* Guest i @guest_bin_start */
        guest = &guests[i];
        regs = &guest->regs;
        guest->vmid = i;
        _guest_module.ops->init(guest, regs);
    }
    printh("[hyp] init_guests: return\n");

    timer_add_callback(TIMER_SCHED, &guest_schedule);
    timer_start(TIMER_SCHED);
}

void start_guest(void)
{
    hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    init_print();
    printh("[%s : %d] Starting...\n", __func__, __LINE__);

    /* Initialize Memory Management */
    ret = hvmm_mm_init();
    if (ret != HVMM_STATUS_SUCCESS)
        printh("[start_guest] virtual memory initialization failed...\n");

    /* Initialize Interrupt Management */
    ret = hvmm_interrupt_init();
    if (ret != HVMM_STATUS_SUCCESS)
        printh("[start_guest] interrupt initialization failed...\n");

    /* Initialize Timer */
    timer_init(TIMER_SCHED);
    /* 100Mhz -> 1 count == 10ns at RTSM_VE_CA15, fast model*/
    timer_set_interval(TIMER_SCHED, GUEST_SCHED_TICK);

    /* Initialize Guests */
    guest_init();

    /* Initialize Virtual Devices */
    ret = vdev_init();
    if (ret != HVMM_STATUS_SUCCESS)
        printh("[start_guest] virtual device initialization failed...\n");

    /* Begin running test code for newly implemented features */
    hvmm_tests_main();

    /* Print Banner */
    printH("%s", BANNER_STRING);

    /* Switch to the first guest */
    guest_sched_start();

    /* The code flow must not reach here */
    printh("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n");
    hyp_abort_infinite();
}
