#include <guest_monitor.h>
#include <monitor_cli.h>
#include <log/string.h>
#include <log/uart_print.h>
#include <asm-arm_inline.h>
#define DEBUG
#include <log/print.h>
#include <gic.h>
#include <guestloader_common.h>
#ifdef _GDB_
#include <gdb_stub.h>
#endif
struct system_map system_maps_code[18000];
uint32_t num_symbols_code;
struct arch_regs target_regs;
uint32_t target_sp;

#define MAX_LENGTH_SYMBOL 100
#define KEY_NOT_FOUND -1
int symbol_binary_search(struct system_map map[], int key, int imin, int imax)
{
    int imid;
    while (imax >= imin) {
        imid = (imin + imax) / 2;
        if (map[imid].address == key)
            return imid;
        else if (map[imid].address < key)
            imin = imid + 1;
        else
            imax = imid - 1;
    }
    if (map[imid].address > key)
        return imid - 1;
    else
        return imid;
}

/*
 * return cnt : The number of type T symbol
 */
uint32_t number_symbol(void)
{
    /* hard coding */
    char last[MAX_LENGTH_SYMBOL];
    int i;
    unsigned char *base;
    uint32_t n_symbol;
    uint32_t cnt;

#ifdef MONITOR_GUEST
    base = (unsigned char *)&system_map_start;
#else
    base = 0;
#endif
    n_symbol = 0;
    cnt = 0;

    while (1) {
        while (*base != ' ') {
            /* address */
            base++;
        }
        /* space */
        base++;
        /* type */
        if (*base == 't' || *base == 'T')
            cnt++;
        base++;
        /* space */
        base++;
        i = 0;
        while (*base != '\r' &&  *base != '\n') {
            /* symbol */
            last[i++] = *base++;
        }
        last[i] = '\0';
        /* carriage return */
        base++;
        n_symbol++;
        if (strcmp(last, "_end") == 0)
            break;
    }
    printh("pre cnt %d\n", cnt);
    return cnt;
}

void symbol_parser_init(void)
{
    /* hard coding */
    /* memory alloc needed to modify TODO : inkyu */
    /*
        printh("num_symbols_code is %d\n", num_symbols_code);
        struct system_map* system_maps = (struct system_map *)
            memory_alloc(n_symbol * sizeof(struct system_map));
     */
    uint8_t *base;
    int cnt;
    int cnt_code;
    int i;
    char address[9];
    char last[MAX_LENGTH_SYMBOL];

    cnt = 0;
    cnt_code = 0;
#ifdef MONITOR_GUEST
    base =  (uint8_t *)&system_map_start;
#else
    base = 0;
#endif

    while (1) {
        i = 0;
        while (*base != ' ') {
            /* address */
            address[i++] = *base;
            base++;
        }
        address[i] = '\0';
        system_maps_code[cnt_code].address =
            (uint32_t)arm_hexstr2uint((char *)address);
        /* space */
        base++;
        /* type */
        system_maps_code[cnt_code].type = *base;
        base++;
        /* space */
        base++;
        i = 0;
        while (*base != '\r' &&  *base != '\n') {
            /* symbol */
            system_maps_code[cnt_code].symbol[i] = last[i] = *base;
            base++;
            i++;
        }
        last[i] = '\0';
        system_maps_code[cnt_code].symbol[i] = '\0';
        /* carriage return */
        base++;
        if (strcmp(last, "_end") == 0)
            break;
        cnt++;
        if (system_maps_code[cnt_code].type == 't' ||
                system_maps_code[cnt_code].type == 'T')
            cnt_code++;
    }
}

void show_symbol(uint32_t va)
{
    int i = 0;
    for (i = 0; i < num_symbols_code; i++)
        ;
        printH("%x %c %s\n", system_maps_code[i].address,
                system_maps_code[i].type, system_maps_code[i].symbol);
}

int symbol_getter_from_va(uint32_t va, char *symbol)
{
    int i;
    i = symbol_binary_search(system_maps_code, va, 0, num_symbols_code - 1);
    if (i == KEY_NOT_FOUND) {
        printH("KEY NOT FOUND\n");
        return KEY_NOT_FOUND;
    }

    strcpy(symbol, (char *)system_maps_code[i].symbol);

    if (system_maps_code[i].address != va) {
        char add[10];
        char op[6] = " + 0x";
        arm_uint2hexstr(add, va - system_maps_code[i].address);
        strcat(symbol, op);
        strcat(symbol, add);
    }
    return 0;
}

#define LIST 0
#define MONITORING 1
#define MEMORY 2
#define REGISTER 3
#define BREAK 4

/* size 200..0xc8 -> 0xEC00100 : memory dump, 0xEC000D0 : vmid info*/
struct monitoring_data {
    uint8_t type;
    uint32_t caller_va;
    uint32_t callee_va;
    uint32_t inst;
    uint32_t memory_range;
    uint32_t start_memory;
    uint8_t monitor_cnt;
    struct vcpu guest;
};

void send_monitoring_data(uint32_t range, uint32_t src)
{
    volatile struct monitoring_data *shared_start =
        (struct monitoring_data *)(&shared_memory_start);

    shared_start->memory_range = range;
    shared_start->start_memory = src;
}

int recovery_cnt;

void set_recovery(int cnt)
{
    recovery_cnt = cnt;
}

void monitoring_handler(int irq, void *pregs, void *pdata)
{
    char call_symbol[MAX_LENGTH_SYMBOL];
    char callee_symbol[MAX_LENGTH_SYMBOL];

    struct monitoring_data *shared_start = (struct monitoring_data
            *)(&shared_memory_start);
    int i;
    if (shared_start->type == MONITORING) {
        symbol_getter_from_va(shared_start->caller_va, call_symbol);
        symbol_getter_from_va(shared_start->callee_va, callee_symbol);
        printh("CPU 0 %s <- %s\n", call_symbol, callee_symbol);

        if (strcmp(call_symbol, "panic") == 0 && recovery_cnt == 1) {
            recovery_cnt = 0;
            printh("Target guest's panic occured!\n");
            printh("Auto system recovery start...\n");
            monitoring_reboot();
        }

    } else if (shared_start->type == LIST) {
        uint32_t *dump_base = (uint32_t *)(&shared_memory_start) + (0x100/4);
        uint32_t va, inst;

        for (i = 0; i < shared_start->monitor_cnt; i++) {
            va = *dump_base;
            symbol_getter_from_va(va, call_symbol);
            dump_base++;
            inst = *dump_base;
            /* showing list */
            printh("Monitoring symbol is %s, va is %x, instruction is %x\n",
                     call_symbol, va, inst);
        }
    } else if (shared_start->type == MEMORY) {
        /* dump memory */
#if 1
        if(get_guest_mode() == MONITORSTUB) {
            int i, j;
            uint32_t *dump_base = (uint32_t *)(&shared_memory_start) + (0x100/4);
            uint32_t base_memory = shared_start->start_memory;

            for (i = 0; i < shared_start->memory_range / 4; i++) {
                if ((uint32_t)dump_base > (uint32_t)&shared_memory_end) {
                    printh("The memory range out!!\n");
                    return;
                }
                if (i % 4 == 0)
                    printH("0x%x :", base_memory);
                printh(" 0x%x", *dump_base);
                dump_base++;
                if (i % 4 == 3) {
                    printh("\n");
                    base_memory += 16;
                }
            }
            printh("\n");
        }

#endif
    } else if (shared_start->type == REGISTER) {
        struct arch_regs regs = shared_start->guest.regs;
        int i;
        target_regs.cpsr = regs.cpsr;
        target_regs.pc = regs.pc;
        // linux
        target_regs.lr = shared_start->guest.context.regs_banked.lr_svc;
        // bmguest
        // target_regs.lr = regs.lr;
        for (i = 0; i < ARCH_REGS_NUM_GPR; i++) {
            target_regs.gpr[i] = regs.gpr[i];
        }
        //linux
        target_sp = shared_start->guest.context.regs_banked.sp_svc;
        //bmguest
        //target_sp = shared_start->guest.context.regs_banked.sp_usr;

#if 1
        if(get_guest_mode() == MONITORSTUB) {
            printH("Cpsr is %x pc is %x lr is %x sp-svc is %x\n", target_regs.cpsr, target_regs.pc, target_regs.lr, shared_start->guest.context.regs_banked.sp_svc);
            printH("Each register is \n");
            for (i = 0; i < ARCH_REGS_NUM_GPR; i++) {
                printh("r%d : %x ", i, target_regs.gpr[i]);
                if(i % 4 == 0 && i != 0)
                    printh("\n");
            }
            printh("\n");
        }
#endif
    } else if (shared_start->type == BREAK) {
        // break target
#ifdef _GDB_
        reply_signal_from_break();
#endif
    }
}



void get_general_reg(struct arch_regs *regs, uint32_t *sp)
{
    int i;
    monitoring_register();
    isb();
    dsb();
    regs->cpsr = target_regs.cpsr;
    regs->pc = target_regs.pc;
    regs->lr = target_regs.lr;
    for (i = 0; i < ARCH_REGS_NUM_GPR; i++) {
        regs->gpr[i] = target_regs.gpr[i];
    }
    *sp = target_sp;
}

void allset(void)
{
    int i = 0;
    for (i = 0; i < num_symbols_code; i++)
        monitoring_allset(system_maps_code[i].address);
}

#define MONITORING_IRQ 20

void reboot(void)
{
    copy_image_to(&restore_start, &restore_end, &loader_start);

    uart_print("\nGuest Linux reboot!!\n");
    loader_boot_guest(GUEST_TYPE);
}

void monitoring_init(void)
{
#ifdef MONITOR_GUEST
    gic_set_irq_handler(MONITORING_IRQ, monitoring_handler, 0);
    num_symbols_code =  number_symbol();
    symbol_parser_init();

#ifdef LINUX_GUEST
    /* For reboot */
    /* TODO Not arndale board yet */
    /* copy_image_to(&loader_start, &guestloader_end, &restore_start); */
#endif

#endif
}

static int guest_mode = 0;

int get_guest_mode(void)
{
    return guest_mode;
}

void set_guest_mode(int mode)
{
    guest_mode = mode;
}
