#include <k-hypervisor-config.h>
#include <vcpu.h>
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

//static struct vcpu guests[NUM_GUESTS_STATIC];
static int _current_guest_vmid[NUM_CPUS] = {VMID_INVALID, VMID_INVALID};
static int _next_guest_vmid[NUM_CPUS] = {VMID_INVALID, };
struct vcpu *_current_guest[NUM_CPUS];
/* further switch request will be ignored if set */
static uint8_t _switch_locked[NUM_CPUS];

static hvmm_status_t guest_save(struct vcpu *vcpu,
                        struct arch_regs *regs)
{
    /* vcpu.hw_save : save the current guest's context*/
    if (_guest_module.ops->save)
        return  _guest_module.ops->save(vcpu, regs);

    return HVMM_STATUS_UNKNOWN_ERROR;
}

static hvmm_status_t guest_restore(struct vcpu *vcpu,
                        struct arch_regs *regs)
{
    /* vcpu.hw_restore : The next becomes the current */
    if (_guest_module.ops->restore)
        return  _guest_module.ops->restore(vcpu, regs);



     return HVMM_STATUS_UNKNOWN_ERROR;
}


static hvmm_status_t perform_switch(struct arch_regs *regs, vcpuid_t next_vmid)
{
    /* _curreng_guest_vmid -> next_vmid */

    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    uint32_t cpu = smp_processor_id();

    if (_current_guest_vmid[cpu] == next_vmid)
        return HVMM_STATUS_IGNORED; /* the same guest? */

    save_and_restore(_current_guest_vmid[cpu], next_vmid, regs);

    return result;
}

hvmm_status_t guest_perform_switch(struct arch_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t cpu = smp_processor_id();

    if (_current_guest_vmid[cpu] == VMID_INVALID) {
        /*
         * If the scheduler is not already running, launch default
         * first guest. It occur in initial time.
         */
        printh("context: launching the first guest\n");

        /* TODO: (igkang) to be modified
         * mananging it by setting timer event
         */
        /* Round-robin code */
        vcpu_reset_tick(_next_guest_vmid[cpu]);

        result = perform_switch(0, _next_guest_vmid[cpu]);
        /* DOES NOT COME BACK HERE */
    } else if (_next_guest_vmid[cpu] != VMID_INVALID &&
                _current_guest_vmid[cpu] != _next_guest_vmid[cpu]) {
        printh("curr: %x\n", _current_guest_vmid[cpu]);
        printh("next: %x\n", _next_guest_vmid[cpu]);

        /* Round-robin code */
        vcpu_reset_tick(_next_guest_vmid[cpu]);
        
        /* Only if not from Hyp */
        result = perform_switch(regs, _next_guest_vmid[cpu]);
        _next_guest_vmid[cpu] = VMID_INVALID;
    }

    /* TODO:(igkang) record start time - for calculating running time
    *   It would be better to place code in a separate function
    */

    _switch_locked[cpu] = 0;
    return result;
}

/* Switch to the first guest */
void guest_sched_start(void)
{
    struct vcpu *vcpu = 0;
    uint32_t cpu = smp_processor_id();

    printh("[hyp] switch_to_initial_guest:\n");
    /* Select the first guest context to switch to. */
    _current_guest_vmid[cpu] = VMID_INVALID;
    if (cpu)
        vcpu = &vcpu_arr[num_of_guest(cpu - 1) + 0];
    else
        vcpu = &vcpu_arr[0];
    /* vcpu.hw_dump */
    if (_guest_module.ops->dump)
        _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_0, &vcpu->regs);
    /* Context Switch with current context == none */

    guest_switchto(vcpu->vmid, 0);
    guest_perform_switch(&vcpu->regs);
}

vcpuid_t guest_first_vmid(void)
{
    uint32_t cpu = smp_processor_id();

    /* FIXME:Hardcoded for now */
#if _SMP_
    if (cpu)
        return 2;
    else
        return 0;
#endif
    return cpu;
}

vcpuid_t guest_last_vmid(void)
{
    uint32_t cpu = smp_processor_id();

    /* FIXME:Hardcoded for now */
#if _SMP_
    if (cpu)
        return 3;
    else
        return 1;
#endif
    //return cpu;
    return NUM_GUESTS_STATIC - 1;
}

vcpuid_t guest_next_vmid(vcpuid_t ofvmid)
{
    vcpuid_t next = VMID_INVALID;
#if 0
#ifdef _SMP_
    uint32_t cpu = smp_processor_id();

    if (cpu)
        return 2;
    else
        return 0;
#endif
#endif

    /* FIXME:Hardcoded */
    if (ofvmid == VMID_INVALID) {
        next = guest_first_vmid();
        return next;
    }
    

    /* FIXME: rename vcpu_tick_plus_one --> vcpu_dec_tick */
    /* Round-robin code */
    vcpu_tick_plus_one(ofvmid);
    if (!vcpu_get_tick(ofvmid)) {
        if (ofvmid < guest_last_vmid()) {
            /* FIXME:Hardcoded */
            next = ofvmid + 1;
        } else {
            next = guest_first_vmid();
        }
    } else {
        next = ofvmid;
    }

    return next;
}

vcpuid_t guest_current_vmid(void)
{
    uint32_t cpu = smp_processor_id();
    return _current_guest_vmid[cpu];
}

vcpuid_t guest_waiting_vmid(void)
{
    uint32_t cpu = smp_processor_id();
    return _next_guest_vmid[cpu];
}

void guest_dump_regs(struct arch_regs *regs)
{
    /* vcpu.hw_dump */
    _guest_module.ops->dump(GUEST_VERBOSE_ALL, regs);
}

hvmm_status_t guest_switchto(vcpuid_t vmid, uint8_t locked)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t cpu = smp_processor_id();

    /* valid and not current vmid, switch */
    if (_switch_locked[cpu] == 0) {
        
        /* TODO: (igkang) calculate&add ran time of current vcpu */

        _next_guest_vmid[cpu] = vmid;
        result = HVMM_STATUS_SUCCESS;
        printh("switching to vmid: %x\n", (uint32_t)vmid);
    } else
        printh("context: next vmid locked to %d\n", _next_guest_vmid[cpu]);

    if (locked)
        _switch_locked[cpu] = locked;

    return result;
}

/* these global var. and functions are for monitor
 * (to fix running guest in scheduler code)
 */
static int manually_next_vmid;
vcpuid_t selected_manually_next_vmid;

void set_manually_select_vmid(vcpuid_t vmid)
{
    manually_next_vmid = 1;
    selected_manually_next_vmid = vmid;
}

void clean_manually_select_vmid(void){
    manually_next_vmid = 0;
}

// (igkang) to be moved into schedalgo-specific file
vcpuid_t sched_policy_determ_next(void)
{
#if 1
    if (manually_next_vmid)
        return selected_manually_next_vmid;
   
   /* TODO: (igkang) to be modified into runqueue style
    * using en/dequeue of certain scheduling algorithm
    */
    vcpuid_t next = guest_next_vmid(guest_current_vmid());

    /* FIXME:Hardcoded */
    if (next == VMID_INVALID)
        next = guest_first_vmid();

    return next;
#endif
//    return guest_first_vmid();
}

void guest_schedule(void *pdata)
{
    struct arch_regs *regs = pdata;
    uint32_t cpu = smp_processor_id();
    vcpuid_t next_vcpuid = guest_current_vmid();

    /* vcpu.hw_dump */
    if (_guest_module.ops->dump)
        _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_3, regs);
    /*
     * Note: As of guest_switchto() and guest_perform_switch()
     * are available, no need to test if trapped from Hyp mode.
     * guest_perform_switch() takes care of it
     */

    next_vcpuid = sched_policy_determ_next();
    
    /* Switch request, actually performed at trap exit */
    guest_switchto(next_vcpuid, 0);

}

hvmm_status_t guest_init()
{
    struct timer_val timer;
    hvmm_status_t result = HVMM_STATUS_SUCCESS;
    struct vcpu *vcpu;
    struct arch_regs *regs = 0;
    int i;
    int guest_count;
    int start_vmid = 0;
    uint32_t cpu = smp_processor_id();
    printh("[hyp] init_guests: enter\n");
    /* Initializes guests */
    guest_count = num_of_guest(cpu);


    if (cpu)
        start_vmid = num_of_guest(cpu - 1);
    else
        start_vmid = 0;

    guest_count += start_vmid;

    for (i = start_vmid; i < guest_count; i++) {
        /* Guest i @guest_bin_start */
        vcpu = &vcpu_arr[i];
        regs = &vcpu->regs;
        vcpu->vmid = i;
        /* vcpu.hw_init */
        if (_guest_module.ops->init)
            _guest_module.ops->init(vcpu, regs);
    }

    printh("[hyp] init_guests: return\n");

    /* 100Mhz -> 1 count == 10ns at RTSM_VE_CA15, fast model*/
    timer.interval_us = GUEST_SCHED_TICK;
    timer.callback = &guest_schedule;

    result = timer_set(&timer, HOST_TIMER);

    if (result != HVMM_STATUS_SUCCESS)
        printh("[%s] timer startup failed...\n", __func__);

    return result;
}

struct vcpu get_guest(uint32_t guest_num)
{
   return vcpu_arr[guest_num];
}

void guest_copy(struct vcpu *dst, vcpuid_t vmid_src)
{
    _guest_module.ops->move(dst, &(vcpu_arr[vmid_src]));
}

void reboot_guest(vcpuid_t vmid, uint32_t pc,
        struct arch_regs **regs)
{
    _guest_module.ops->init(&vcpu_arr[vmid], &(vcpu_arr[vmid].regs));
    vcpu_arr[vmid].regs.pc = pc;
    vcpu_arr[vmid].regs.gpr[10] = 1;
    if (regs != 0)
        _guest_module.ops->restore(&vcpu_arr[vmid], *regs);
}

void save_and_restore(vcpuid_t from, vcpuid_t to, struct arch_regs *regs){

    struct vcpu *vcpu = 0;
    uint32_t cpu = smp_processor_id();

    guest_save(&vcpu_arr[from], regs);
    memory_save();
    interrupt_save(from);
    vdev_save(from);

    /* The context of the next guest */
    vcpu = &vcpu_arr[to];
    _current_guest[cpu] = vcpu;
    _current_guest_vmid[cpu] = to;

    /* vcpu.hw_dump */
    if (_guest_module.ops->dump)
        _guest_module.ops->dump(GUEST_VERBOSE_LEVEL_3, &vcpu->regs);

    vdev_restore(to);
    interrupt_restore(to);
    memory_restore(to);
    guest_restore(vcpu, regs);
}
void vcpu_init(){
    int i = 0;
    struct vcpu *vcpu = 0;
    for(i = 0 ; i < NUM_GUESTS_STATIC ; i++){
        vcpu = &vcpu_arr[i];

        /* FIXME: Hardcoded - tick */
        /* Round-robin code */
        vcpu->tick_reset_val = 5;
    }
}
void vcpu_change_state(vcpuid_t vcpu_id, vcpu_state_t state){
    vcpu_arr[vcpu_id].vcpu_state = state;
}

uint32_t vcpu_get_tick(vcpuid_t vcpu_id){
    return vcpu_arr[vcpu_id].tick;
}

void vcpu_reset_tick(vcpuid_t vcpu_id)
{
    struct vcpu *vcpu = 0;
    vcpu = &vcpu_arr[vcpu_id];

    vcpu->tick = vcpu->tick_reset_val;
}

void vcpu_tick_plus_one(vcpuid_t vcpu_id){
    struct vcpu *vcpu = 0;
    vcpu = &vcpu_arr[vcpu_id];

    vcpu->tick--;
}

void vcpu_running_time_plus_one(vcpuid_t vcpu_id){
    vcpu_arr[vcpu_id].running_time++;
}

void vcpu_actual_running_time_plus_one(vcpuid_t vcpu_id){
    vcpu_arr[vcpu_id].actual_running_time++;
}

