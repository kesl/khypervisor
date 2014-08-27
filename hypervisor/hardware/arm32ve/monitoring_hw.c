#include <monitoring.h>
#include <interrupt.h>
#include <asm_io.h>
#include <guest.h>
#include <log/print.h>

#define l1_tbl_index_offset 20
#define l2_tbl_index_offset 12
#define l1_tbl_index(va)   va >> l1_tbl_index_offset
#define l2_tbl_index(va)   va >> (l2_tbl_index_offset & 0xFF)

#define l1_descriptor_address(trans_base, l1_table_index) \
            trans_base | (l1_table_index << 2)

#define l1_descriptor(trans_base, l1_table_index) \
            readl(l1_descriptor_address(trans_base, l1_table_index))

#define l2_descriptor_address(page_table_base_address, l2_table_index) \
            page_table_base_address | (l2_table_index << 2)

#define l2_descriptor(page_table_base_address, l2_table_index) \
            readl(l2_descriptor_address(page_table_base_address, l2_table_index))

uint64_t va_to_pa(uint32_t va, uint32_t ttbr_num)
{
    
    uint32_t linux_guest_ttbr, l1_des, l2_des;
    if(ttbr_num){
        uint32_t linux_guest_ttbr1 = get_guest(0).context.regs_cop.ttbr1;
        linux_guest_ttbr = linux_guest_ttbr1;
    }
    else{
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

    switch(l1_des & 0b11) {
    case 0b00:
        /* Fault */
        printH("[%s : %d] l1 page fault\n", __func__, __LINE__);
        return 0;
    case 0b01:
        /* page table */
        l2_des = l2_descriptor((l1_des & 0xFFFFFC00), l2_tbl_index(va));
        switch(l2_des & 0b11){
            case 0b00:
                /* Fault */
                printh("[%s : %d] l2 page fault\n", __func__, __LINE__);
                return 0;
            case 0b01:
                /* Large page */
                printh("[%s : %d] Large page table\n", __func__, __LINE__);
                return ((l2_des & 0xFFFF0000) | (va & 0xFFFF));
            case 0b10:
            case 0b11:
                /* Small page */
                printh("[%s : %d] Small page table\n", __func__, __LINE__);
                return ((l2_des & 0xFFFFF000) | (va & 0xFFF));
        }
        break;
    case 0b10:
        /* Supersection */
        if(l1_des & (1 << 18)){
        printh("[%s : %d] Supersection\n", __func__, __LINE__);
           return (((l1_des & 0x1E0) << 23 ) | ((l1_des & 0x00F00000) << 4) | 
            (l1_des & 0xFF000000) | (va & 0x00FFFFFF));
        }
        /* Section */
        else{
        printh("[%s : %d] Section\n", __func__, __LINE__);
            return ((l1_des & 0xFFF00000) | (va & 0x000FFFFF));
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

void show_symbole(uint32_t va)
{
    unsigned char *base = &system_map_start;
    int i;
    printH("%s\n", base);
//    for(i = 0; i < 100; i++)
//        printH("%c", *base++);
}

uint32_t va_getter_from_symbol(char * symbol)
{
    return 0;
}


