#include <monitoring.h>
#include <interrupt.h>
#include <asm_io.h>
#include <guest.h>
#include <log/print.h>
#include <memory.h>
#include <log/string.h>

#define l1_tbl_index_offset 20
#define l2_tbl_index_offset 12
#define l1_tbl_index(va)   (va >> l1_tbl_index_offset)
#define l2_tbl_index(va)   (va >> (l2_tbl_index_offset & 0xFF))

#define l1_descriptor_address(trans_base, l1_table_index) \
        (trans_base | (l1_table_index << 2))

#define l1_descriptor(trans_base, l1_table_index) \
        readl(l1_descriptor_address(trans_base, l1_table_index))

#define l2_descriptor_address(page_table_base_address, l2_table_index) \
        (page_table_base_address | (l2_table_index << 2))

#define l2_descriptor(page_table_base_address, l2_table_index) \
        readl(l2_descriptor_address(page_table_base_address, l2_table_index))

uint64_t va_to_pa(uint32_t va, uint32_t ttbr_num)
{
    uint32_t linux_guest_ttbr, l1_des, l2_des;
    if (ttbr_num) {
        uint32_t linux_guest_ttbr1 = get_guest(0).context.regs_cop.ttbr1;
        linux_guest_ttbr = linux_guest_ttbr1;
    } else {
        uint32_t linux_guest_ttbr0 = get_guest(0).context.regs_cop.ttbr0;
        linux_guest_ttbr = linux_guest_ttbr0;
    }
    printh("va_to_pa start===== ttbr is %x, va is %x\n", linux_guest_ttbr, va);
    /*
     * If ttbcr.N is not 0, it needs to modify.
     * (0x3FFFF >> ttbcr.n) << (14 + ttbcr.n)
     */
    l1_des = l1_descriptor((linux_guest_ttbr & 0xFFFFC000), l1_tbl_index(va));
    printh("l1_des is %x\n", l1_des);

    switch (l1_des & 0b11) {
    case 0b00:
        /* Fault */
        printH("[%s : %d] l1 page fault\n", __func__, __LINE__);
        return 0;
    case 0b01:
        /* page table */
        l2_des = l2_descriptor((l1_des & 0xFFFFFC00), l2_tbl_index(va));
        switch (l2_des & 0b11) {
        case 0b00:
            /* Fault */
            printh("[%s : %d] l2 page fault\n", __func__, __LINE__);
            return 0;
        case 0b01:
            /* Large page */
            printh("[%s : %d] Large page table\n", __func__, __LINE__);
            return (l2_des & 0xFFFF0000) | (va & 0xFFFF);
        case 0b10:
        case 0b11:
            /* Small page */
            printh("[%s : %d] Small page table\n", __func__, __LINE__);
            return (l2_des & 0xFFFFF000) | (va & 0xFFF);
        }
        break;
    case 0b10:
        /* Supersection */
        if (l1_des & (1 << 18)) {
            printh("[%s : %d] Supersection\n", __func__, __LINE__);
            return ((l1_des & 0x1E0) << 23) | ((l1_des & 0x00F00000) << 4) |
            (l1_des & 0xFF000000) | (va & 0x00FFFFFF);
        } else {
        /* Section */
            printh("[%s : %d] Section\n", __func__, __LINE__);
            return (l1_des & 0xFFF00000) | (va & 0x000FFFFF);
        }
        break;
    case 0b11:
        /* Reserved */
        printH("[%s : %d] reserved\n", __func__, __LINE__);
        return 0;
    default:
        printH("[%s : %d] ERROR\n", __func__, __LINE__);
        return 0;
    }
    return 0;
}

struct system_map system_maps[32323];
uint32_t num_symbols;

#define KEY_NOT_FOUND -1
int symbol_binary_search(struct system_map M[], int key, int imin, int imax)
{
    int imid;
    while (imax >= imin) {
        imid = (imin + imax) / 2;
        if (M[imid].address == key)
            return imid;
        else if (M[imid].address < key)
            imin = imid + 1;
        else
            imax = imid - 1;
    }
    return imid;
}

int symbol_near_search(struct system_map M[], int key, int imin, int imax)
{
    int imid;
    printH("ket is %x\n", key);
    while (imax >= imin) {
        imid = (imin + imax) / 2;
        if (M[imid].address == key)
            return imid;
        else if (M[imid].address < key) {
            if (imax - imin == 1)
                return imin;
            imin = imid;
        } else
            imax = imid;
    }
    return imid;
}

uint32_t number_symbol(void)
{
    /* hard coding */
    unsigned char *base = 0;
#ifdef _MON_
    *base = (unsigned char *)&system_map_start;
#endif
    uint32_t n_symbol = 0;
    char last[50];
    int i;
    while (1) {
        while (*base != ' ') {
            /* address */
            base++;
        }
        /* space */
        /* type */
        /* space */
        base = base + 3;
        for (i = 0; i < 50; i++)
            last[0] = '\0';
        i = 0;
        while (*base != '\r' &&  *base != '\n') {
            /* symbole */
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
}

uint32_t get_num_symbol(void)
{
    return num_symbols;
}

struct system_map *get_symbols(void)
{
    return system_maps;
}

void monitor_init(void)
{
    /* hard coding */
    num_symbols = number_symbol();
    /* memory alloc needed to modify TODO : inkyu */
    /*
    struct system_map* system_maps =
        (struct system_map *)memory_alloc(n_symbol * sizeof(struct system_map));
        */
    uint8_t *base = 0;
#ifdef _MON_
    *base = (uint8_t *)&system_map_start;
#endif
    char last[50];
    int cnt = 0;
    int i;
    char address[9];
    while (1) {
        i = 0;
        while (*base != ' ') {
            /* address */
            address[i++] = *base;
            base++;
        }
       address[i] = '\0';
       system_maps[cnt].address = (uint32_t)arm_hexstr2uint((char *)address);
        /* space */
        base++;
        /* type */
        system_maps[cnt].type = *base;
        base++;
        /* space */
        base++;

        for (i = 0; i < 50; i++)
            last[0] = '\0';
        i = 0;
        while (*base != '\r' &&  *base != '\n') {
            /* symbol */
            system_maps[cnt].symbol[i] = last[i] = *base;
            base++;
            i++;
        }
        system_maps[cnt].symbol[i] = '\0';
        last[i] = '\0';
        /* carriage return */
        base++;
        if (strcmp(last, "_end") == 0)
            break;
        cnt++;
    }
}

void show_symbole(uint32_t va)
{
    int i = 0;
    for (i = 0; i < num_symbols; i++)
        printH("%x %c %s\n", system_maps[i].address, system_maps[i].type,
                system_maps[i].symbol);
}

int symbol_getter_from_va(uint32_t va, char **symbol)
{
    int i;
    i = symbol_binary_search(system_maps, va, 0, num_symbols-1);
    if (i == KEY_NOT_FOUND) {
        printH("KEY NOT FOUND\n");
        return KEY_NOT_FOUND;
    }
    *symbol = (char *)system_maps[i].symbol;
    return 0;
}

int symbol_getter_from_va_lr(uint32_t va, char **symbol)
{
    int i;
    i = symbol_near_search(system_maps, va, 0, num_symbols-1);
    if (i == KEY_NOT_FOUND) {
        printH("KEY NOT FOUND\n");
        return KEY_NOT_FOUND;
    }
    *symbol = (char *)system_maps[i].symbol;
    return 0;
}

uint32_t va_getter_from_symbol(char *symbol)
{
    return 0;
}
