#ifndef __GUEST_MONITOR_H__
#define __GUEST_MONITOR_H__

#include <arch_types.h>
#define MAX_LENGTH_SYMBOL 50

#define VDEV_MONITORING_BASE 0x3FFFD000

#define MONITOR_READ_LIST                   (0x00)
#define MONITOR_READ_RUN                    (0x01 * 4)
#define MONITOR_READ_CEAN_ALL               (0x02 * 4)
#define MONITOR_READ_DUMP_MEMORY            (0x03 * 4)
#define MONITOR_WRITE_TRACE_GUEST           (0x04 * 4)
#define MONITOR_WRITE_CLEAN_TRACE_GUEST     (0x05 * 4)
#define MONITOR_WRITE_BREAK_GUEST           (0x06 * 4)
#define MONITOR_WRITE_CLEAN_BREAK_GUEST     (0x07 * 4)
#define MONITOR_READ_REBOOT                 (0x08 * 4)
#define MONITOR_READ_RECOVERY               (0x09 * 4)
#define MONITOR_READ_REGISTER               (0x0a * 4)
#define MONITOR_READ_STOP                   (0x0b * 4)
#define MONITOR_READ_PUT_MEMORY             (0x0c * 4)

#define GDBSTUB 1
#define MONITORSTUB 2

struct system_map {
    uint32_t address;
    uint8_t type;
    char symbol[MAX_LENGTH_SYMBOL];
};

/* co-processor registers: cp15, cp2 */
struct regs_cop {
    uint32_t vbar;
    uint32_t ttbr0;
    uint32_t ttbr1;
    uint32_t ttbcr;
    uint32_t sctlr;
};


/* banked registers */
struct regs_banked {
    uint32_t sp_usr;
    uint32_t spsr_svc;
    uint32_t sp_svc;
    uint32_t lr_svc;
    uint32_t spsr_abt;
    uint32_t sp_abt;
    uint32_t lr_abt;
    uint32_t spsr_und;
    uint32_t sp_und;
    uint32_t lr_und;
    uint32_t spsr_irq;
    uint32_t sp_irq;
    uint32_t lr_irq;

    uint32_t spsr_fiq;
    /* Cortex-A15 processor does not support sp_fiq */
    /* uint32_t sp_fiq; */
    uint32_t lr_fiq;
    uint32_t r8_fiq;
    uint32_t r9_fiq;
    uint32_t r10_fiq;
    uint32_t r11_fiq;
    uint32_t r12_fiq;
};

/* Defines the architecture specific information, except general regsiters */
struct arch_context {
    struct regs_cop regs_cop;
    struct regs_banked regs_banked;
};

#define ARCH_REGS_NUM_GPR    13
/* Defines the architecture specific registers */
struct arch_regs {
    uint32_t cpsr; /* CPSR */
    uint32_t pc; /* Program Counter */
    uint32_t lr;
    uint32_t gpr[ARCH_REGS_NUM_GPR]; /* R0 - R12 */
} __attribute((packed));


struct vcpu {
    struct arch_regs regs;
    struct arch_context context;
    uint8_t vmid;
};

extern uint32_t shared_memory_start;
extern uint32_t shared_memory_end;
extern uint32_t system_map_start;
void show_symbol(uint32_t va);
void sysmbol_parser_init(void);
int symbol_getter_from_va(uint32_t va, char *symbol);
void monitoring_init(void);
void send_monitoring_data(uint32_t range, uint32_t src);
void reboot(void);
void set_recovery(int cnt);
void allset(void);
void get_general_reg(struct arch_regs *regs, uint32_t *sp);

extern uint32_t loader_start;
extern uint32_t restore_start;
extern uint32_t restore_end;
extern uint32_t guestloader_end;
extern uint32_t restore_guest_start;
extern uint32_t restore_guest_end;
extern uint32_t guest_start;
int get_guest_mode(void);
void set_guest_mode(int mode);
#endif
