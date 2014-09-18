#include <monitoring.h>
#include <monitoring_cli.h>
#include <log/string.h>
#define DEBUG
#include <log/print.h>
#include <gic.h>

struct system_map system_maps_code[18000];
uint32_t num_symbols_code;

#define MAX_LENGTH_SYMBOL 50
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
 * cnt : The number of type T symbol
 * return : The number of total symbol
 */
uint32_t number_symbol(uint32_t *cnt)
{
#ifdef _MON_
    /* hard coding */
    unsigned char *base = 0;
    uint32_t n_symbol = 0;
    char last[MAX_LENGTH_SYMBOL];
    int i;
    base = (unsigned char *)&system_map_start;
    while (1) {
        while (*base != ' ') {
            /* address */
            base++;
        }
        /* space */
        base++;
        /* type */
        if (*base == 't' || *base == 'T')
            (*cnt)++;
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
    return n_symbol;
#endif
}

void symbol_parser_init(void)
{
#ifdef _MON_
    /* hard coding */
    number_symbol(&num_symbols_code);
    printh("%d\n", num_symbols_code);
    /* memory alloc needed to modify TODO : inkyu */
    /*
    struct system_map* system_maps =
        (struct system_map *)memory_alloc(n_symbol * sizeof(struct system_map));
        */
    uint8_t *base = (uint8_t *)&system_map_start;
    int cnt = 0;
    int cnt_code = 0;
    int i;
    char address[9];
    char last[MAX_LENGTH_SYMBOL];
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
#endif
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

#define ARCH_REGS_NUM_GPR    13
/* Defines the architecture specific registers */
struct arch_regs {
    uint32_t cpsr; /* CPSR */
    uint32_t pc; /* Program Counter */
    uint32_t lr;
    uint32_t gpr[ARCH_REGS_NUM_GPR]; /* R0 - R12 */
} __attribute((packed));

struct monitoring_data {
    uint32_t caller_va;
    uint32_t callee_va;
    uint32_t inst;
    struct arch_regs regs;
};

void monitoring_handler(int irq, void *pregs, void *pdata)
{
    char call_symbol[MAX_LENGTH_SYMBOL];
    char callee_symbol[MAX_LENGTH_SYMBOL];

    struct monitoring_data *shared_start = (struct monitoring_data *)0x8EC00000;
    symbol_getter_from_va(shared_start->caller_va, call_symbol);
    if (shared_start->callee_va != 0) {
        symbol_getter_from_va(shared_start->callee_va, callee_symbol);
        printh("CPU 0 %s <- %s\n", call_symbol, callee_symbol);
    } else {
        /* showing list */
        printh("Monitoring symbol is %s, va is %x, instruction is %x\n",
                    call_symbol, shared_start->caller_va, shared_start->inst);
    }
}

#define MONITORING_IRQ 20

void monitoring_init(void)
{
    gic_set_irq_handler(MONITORING_IRQ, monitoring_handler, 0);
    symbol_parser_init();
}

