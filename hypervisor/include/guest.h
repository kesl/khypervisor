#ifndef _GUEST_H__
#define _GUEST_H__

#include "armv7_p15.h"
#include "timer.h"
#include <log/uart_print.h>
#include <hvmm_types.h>
#include <vgic.h>
#include <guest_hw.h>

enum hyp_hvc_result {
    HYP_RESULT_ERET = 0,
    HYP_RESULT_STAY = 1
};

#define GUEST_VERBOSE_ALL       0xFF
#define GUEST_VERBOSE_LEVEL_0   0x01
#define GUEST_VERBOSE_LEVEL_1   0x02
#define GUEST_VERBOSE_LEVEL_2   0x04
#define GUEST_VERBOSE_LEVEL_3   0x08
#define GUEST_VERBOSE_LEVEL_4   0x10
#define GUEST_VERBOSE_LEVEL_5   0x20
#define GUEST_VERBOSE_LEVEL_6   0x40
#define GUEST_VERBOSE_LEVEL_7   0x80

struct guest_struct {
    struct arch_regs regs;
    struct arch_context context;
    vmid_t vmid;
};

struct guest_ops {
    /** Initalize guest state */
    hvmm_status_t (*init)(struct guest_struct *, struct arch_regs *);

    /** Save registers for context switch */
    hvmm_status_t (*save)(struct guest_struct *, struct arch_regs *);

    /** Restore registers for context switch */
    hvmm_status_t (*restore)(struct guest_struct *, struct arch_regs *);

    /** Dump state of the guest */
    hvmm_status_t (*dump)(uint8_t, struct arch_regs *regs);
};

struct guest_module {
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

    /** Guest Operation */
    struct guest_ops *ops;

};

extern uint32_t _guest0_bin_start;
extern uint32_t _guest0_bin_end;
extern uint32_t _guest1_bin_start;
#ifdef _SMP_
extern uint32_t _guest2_bin_start;
extern uint32_t _guest2_bin_end;
extern uint32_t _guest3_bin_start;
extern uint32_t _guest3_bin_end;
#endif
extern struct guest_module _guest_module;
extern struct guest_struct *_current_guest[NUM_CPUS];

/**
 * sched_policy_determ_next() should be used to determine next virtual
 * machin. Currently, K-Hypervisor scheduler is a round robin, so
 * it has been implemented very simply by increasing the vmid number.
 */
vmid_t sched_policy_determ_next(void);

/**
 * guest_perform_switch() perform the exchange of register from old virtual
 * to new virtual machine. Mainly, this function is called by trap and
 * the timer interrupt.
 */
hvmm_status_t guest_perform_switch(struct arch_regs *regs);

void guest_dump_regs(struct arch_regs *regs);
void guest_sched_start(void);
vmid_t guest_first_vmid(void);
vmid_t guest_last_vmid(void);
vmid_t guest_next_vmid(vmid_t ofvmid);
vmid_t guest_current_vmid(void);
vmid_t guest_waiting_vmid(void);
hvmm_status_t guest_switchto(vmid_t vmid, uint8_t locked);
extern void __mon_switch_to_guest_context(struct arch_regs *regs);
hvmm_status_t guest_init();
struct guest_struct get_guest(uint32_t guest_num);
void set_manually_select_vmid(vmid_t vmid);
void clean_manually_select_vmid(void);

static inline unsigned long num_of_guest(int cpu)
{
    if (cpu == 0)
        return NUM_GUESTS_CPU0_STATIC;
    else
        return NUM_GUESTS_CPU1_STATIC;
}

#endif
