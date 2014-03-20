#include <k-hypervisor-config.h>
#include "mm.h"
#include "vmm.h"
#include "armv7_p15.h"
#include "arch_types.h"

#include <config/memmap.cfg>
#include <log/print.h>
#include <log/uart_print.h>

/**
 * \defgroup Memory_Attribute_Indirection_Register
 *
 * It provide the memory attribute encodings corresponding to the possible
 * AttrIndx values in a Long descriptor format translation table entry for
 * stage-1 translations.
 *
 * - Only accessible from PL1 or higher.
 * AttrIndx[2] selects the appropriate MAIR
 * - AttrIndx[2] to 0 selects MAIR0
 *               to 1 selects MAIR1
 * - AttrIndx[1:0] gives the value of m in Attrm
 * - The MAIR0 and MAIR1 bit assignments.
 * <pre>
 * |     |31       24|23       16|15        8|7         0|
 * |-----|-----------|-----------|-----------|-----------|
 * |MAIR0|   Attr3   |   Attr2   |   Attr1   |   Attr0   |
 * |-----|-----------|-----------|-----------|-----------|
 * |MAIR1|   Attr7   |   Attr6   |   Attr5   |   Attr4   |
 * </pre>
 * - MAIRn Attrm[7:4] encoding
 * <pre>
 * |Attrm[7:4]     |Meaning                                                    |
 * |---------------|-----------------------------------------------------------|
 * |0000           |Strongly-ordered or Device memory, see encoding Attrm[3:0] |
 * |---------------|-----------------------------------------------------------|
 * |00RW, RW not 00| - Normal memory, Outer Write-Through transient.           |
 * |---------------|-----------------------------------------------------------|
 * |0100           |Normal memory, Outer non-cacheable.                        |
 * |---------------|-----------------------------------------------------------|
 * |01RW, RW not 00| - Normal memory, Outer Write-back transient.              |
 * |---------------|-----------------------------------------------------------|
 * |10RW           |Normal memory, Outer Write-Through Cacheable, Non-transient|
 * |---------------|-----------------------------------------------------------|
 * |11RW           | Normal memory, Ouet Write-Back Cacheable, Non-transient.  |
 * </pre>
 *
 * - MAIRn Attrm[3:0] encoding
 * <pre>
 * |Attrm[3:0]|When Attrm[7:4] is 0b000|When Attrm[7:4] is not 0b0000
 * |----------|------------------------|--------------------------------------|
 * |0000      |Strongly-ordered memory | UNPREDICTABLE                        |
 * |----------|------------------------|--------------------------------------|
 * |00RW,     | UNPREDICTABLE          | - Normal memory, Inner Write-Through |
 * |RW not 00 |                        |   transient.                         |
 * |----------|------------------------|--------------------------------------|
 * |0100      | Device memory          | Normal memory, Inner Non-cacheable.  |
 * |----------|------------------------|--------------------------------------|
 * |01RW,     | UNPREDICTABLE          | - Normal memory, Inner Write-Back    |
 * |RW not 00 |                        |   Transient.                         |
 * |----------|------------------------|--------------------------------------|
 * |10RW      | UNPREDICTABLE          | Normal memory, Inner Write-Through   |
 * |          |                        | Cacheable, Non-transient.            |
 * |----------|------------------------|--------------------------------------|
 * |11RW      | UNPREDICTABLE          | Normal memory, Inner Write-back      |
 * |          |                        | Cacheable, Non-transient.            |
 * </pre>
 *
 * -R, W bit mean read-allocate and wrtie-allocate.
 *
 * - Used initial value of MAIR0, MAIR1.
 * <pre>
 * |     |31       24|23       16|15        8|7         0|
 * |-----|-----------|-----------|-----------|-----------|
 * |MAIR0| 1110 1110 | 1010 1010 | 0100 0100 | 0000 0000 |
 * |-----|-----------|-----------|-----------|-----------|
 * |MAIR1| 1111 1111 | 0000 0000 | 0000 0000 | 0000 0100 |
 * </pre>
 * @{
 */
#define INITIAL_MAIR0VAL 0xeeaa4400
#define INITIAL_MAIR1VAL 0xff000004
#define INITIAL_MAIRVAL (INITIAL_MAIR0VAL|INITIAL_MAIR1VAL<<32)
/** @}*/

/**
 * \defgroup SCTLR
 *
 * System Control Register.
 * The SCTLR provides the top level control of the system, including its memory
 * system.
 * - HSCTLR is a subset of this
 *
 * @{
 */
#define SCTLR_TE        (1<<30) /**< Thumb Exception enable. */
#define SCTLR_AFE       (1<<29) /**< Access Flag Enable. */
#define SCTLR_TRE       (1<<28) /**< TEX Remp Enable. */
#define SCTLR_NMFI      (1<<27) /**< Non-Maskable FIQ(NMFI) support. */
#define SCTLR_EE        (1<<25) /**< Exception Endianness. */
#define SCTLR_VE        (1<<24) /**< Interrupt Vectors Enable. */
#define SCTLR_U         (1<<22) /**< In ARMv7 this bit is RAO/SBOP. */
#define SCTLR_FI        (1<<21) /**< Fast Interrupts configuration enable. */
#define SCTLR_WXN       (1<<19) /**< Write permission emplies XN. */
#define SCTLR_HA        (1<<17) /**< Hardware Access flag enable. */
#define SCTLR_RR        (1<<14) /**< Round Robin select. */
#define SCTLR_V         (1<<13) /**< Vectors bit. */
#define SCTLR_I         (1<<12) /**< Instruction cache enable.  */
#define SCTLR_Z         (1<<11) /**< Branch prediction enable. */
#define SCTLR_SW        (1<<10) /**< SWP and SWPB enable. */
#define SCTLR_B         (1<<7)  /**< In ARMv7 this bit is RAZ/SBZP. */
#define SCTLR_C         (1<<2)  /**< Cache enable. */
#define SCTLR_A         (1<<1)  /**< Alignment check enable. */
#define SCTLR_M         (1<<0)  /**< MMU enable. */
#define SCTLR_BASE        0x00c50078  /**< STCLR Base address */
#define HSCTLR_BASE       0x30c51878  /**< HSTCLR Base address */
/** @}*/

/**
 * \defgroup HTTBR
 * @brief Hyp Translation Table Base Register
 *
 * The HTTBR holds the base address of the translation table for the stasge-1
 * translation of memory accesses from Hyp mode.
 * @{
 */
#define HTTBR_INITVAL               0x0000000000000000ULL
#define HTTBR_BADDR_MASK            0x000000FFFFFFF000ULL
#define HTTBR_BADDR_SHIFT           12
/** @}*/

/**
 * \defgroup HTCR
 * @brief Hyp Translation Control Register
 *
 * The HTCR controls the translation table walks required for the stage-1
 * translation of memory accesses from Hyp mode, and holds cacheability and
 * shareability information for the accesses.
 * @{
 */
#define HTCR_INITVAL                0x80000000
#define HTCR_SH0_MASK               0x00003000 /**< \ref Shareability */
#define HTCR_SH0_SHIFT              12
#define HTCR_ORGN0_MASK             0x00000C00 /**< \ref Outer_cacheability */
#define HTCR_ORGN0_SHIFT            10
#define HTCR_IRGN0_MASK             0x00000300 /**< \ref Inner_cacheability */
#define HTCR_IRGN0_SHIFT            8
#define HTCR_T0SZ_MASK              0x00000003
#define HTCR_T0SZ_SHIFT             0
/** @} */

/* PL2 Stage 1 Level 1 */
#define HMM_L1_PTE_NUM  512

/* PL2 Stage 1 Level 2 */
#define HMM_L2_PTE_NUM  512

/* PL2 Stage 1 Level 3 */
#define HMM_L3_PTE_NUM  512

#define HEAP_ADDR (CFG_MEMMAP_MON_OFFSET + 0x02000000)
#define HEAP_SIZE 0x0D000000

#define L2_ENTRY_MASK 0x1FF
#define L2_SHIFT 21

#define L3_ENTRY_MASK 0x1FF
#define L3_SHIFT 12

#define HEAP_END_ADDR (HEAP_ADDR + HEAP_SIZE)
#define NALLOC 1024

static union lpaed _hmm_pgtable[HMM_L1_PTE_NUM] \
                __attribute((__aligned__(4096)));
static union lpaed _hmm_pgtable_l2[HMM_L2_PTE_NUM] \
                __attribute((__aligned__(4096)));
static union lpaed _hmm_pgtable_l3[HMM_L2_PTE_NUM][HMM_L3_PTE_NUM] \
                __attribute((__aligned__(4096)));

/* used malloc, free, sbrk */
union header {
    struct {
        union header *ptr; /* next block if on free list */
        unsigned int size; /* size of this block */
    } s;
    /* force align of blocks */
    long x;
};
/* free list block header */

uint32_t mm_break; /* break point for sbrk()  */
uint32_t mm_prev_break; /* old break point for sbrk() */
uint32_t last_valid_address; /* last mapping address */
static union header freep_base; /* empty list to get started */
static union header *freep; /* start of free list */

/**
 * @brief Initilization of heap memory region.
 *
 * Initializes the configure variable of heap region for malloc operation.
 * - mm_break = mm_prev_break = last_valid_address = HEAD_ADDR
 *
 * @return void
 */
void hmm_heap_init(void)
{
    mm_break = HEAP_ADDR;
    mm_prev_break = HEAP_ADDR;
    last_valid_address = HEAP_ADDR;
    freep = 0;
}

/*
 * Initialization of Host Monitor Memory Management
 * PL2 Stage1 Translation
 * VA32 -> PA
 */
/**
 * @brief Initializes the hyp mode memory management.
 *
 * Initialize and setting the translation table descriptors of the Hyp mode.
 * - Target environment.
 *   - PL2, stage-1 translation table.
 *   - Virtual address -> Physical address
 * - Generate configurations.
 *   - Translation table level1, level2, and level3.
 * <pre>
 * Name         Physical address range    Location     Attribute Index Setting
 * Partition 0: 0x00000000 ~ 0x3FFFFFFF - Peripheral - ATTR_IDX_DEV_SHARED
 * Partition 1: 0x40000000 ~ 0x7FFFFFFF - Unused     - ATTR_IDX_UNCACHED
 * Partition 2: 0x80000000 ~ 0xBFFFFFFF - Guest      - ATTR_IDX_UNCACHED
 * Partition 3: 0xC0000000 ~ 0xFFFFFFFF - Hypervisor
 *                                      - Level2 and level3 translation table
 * </pre>
 *
 * @return void
 */
static void _hmm_init(void)
{
    int i, j;
    uint64_t pa = 0x00000000ULL;

    _hmm_pgtable[0] = hvmm_mm_lpaed_l1_block(pa, ATTR_IDX_DEV_SHARED);
    pa += 0x40000000;
    uart_print("&_hmm_pgtable[0]:");
    uart_print_hex32((uint32_t) &_hmm_pgtable[0]);
    uart_print("\n\r");
    uart_print("lpaed:");
    uart_print_hex64(_hmm_pgtable[0].bits);
    uart_print("\n\r");
    _hmm_pgtable[1] = hvmm_mm_lpaed_l1_block(pa, ATTR_IDX_UNCACHED);
    pa += 0x40000000;
    uart_print("&_hmm_pgtable[1]:");
    uart_print_hex32((uint32_t) &_hmm_pgtable[1]);
    uart_print("\n\r");
    uart_print("lpaed:");
    uart_print_hex64(_hmm_pgtable[1].bits);
    uart_print("\n\r");
    _hmm_pgtable[2] = hvmm_mm_lpaed_l1_block(pa, ATTR_IDX_WRITEALLOC);
    pa += 0x40000000;
    uart_print("&_hmm_pgtable[2]:");
    uart_print_hex32((uint32_t) &_hmm_pgtable[2]);
    uart_print("\n\r");
    uart_print("lpaed:");
    uart_print_hex64(_hmm_pgtable[2].bits);
    uart_print("\n\r");
    /* _hmm_pgtable[3] refers Lv2 page table address. */
    _hmm_pgtable[3] = hvmm_mm_lpaed_l1_table((uint32_t) _hmm_pgtable_l2);
    uart_print("&_hmm_pgtable[3]:");
    uart_print_hex32((uint32_t) &_hmm_pgtable[3]);
    uart_print("\n\r");
    uart_print("lpaed:");
    uart_print_hex64(_hmm_pgtable[3].bits);
    uart_print("\n\r");
    for (i = 0; i < HMM_L2_PTE_NUM; i++) {
        /*
         * _hvmm_pgtable_lv2[i] refers Lv3 page table address.
         * each element correspond 2MB
         */
        _hmm_pgtable_l2[i] =
                hvmm_mm_lpaed_l2_table((uint32_t) _hmm_pgtable_l3[i]);
        /* _hvmm_pgtable_lv3[i][j] refers page, that size is 4KB */
        for (j = 0; j < HMM_L3_PTE_NUM; pa += 0x1000, j++) {
            /* 0xF2000000 ~ 0xFF000000 - Heap memory 208MB */
            if (pa >= HEAP_ADDR && pa < HEAP_ADDR + HEAP_SIZE) {
                _hmm_pgtable_l3[i][j] =
                        hvmm_mm_lpaed_l3_table(pa, ATTR_IDX_WRITEALLOC, 0);
            } else {
                _hmm_pgtable_l3[i][j] =
                        hvmm_mm_lpaed_l3_table(pa, ATTR_IDX_UNCACHED, 1);
            }
        }
    }
    for (i = 4; i < HMM_L1_PTE_NUM; i++)
        _hmm_pgtable[i].pt.valid = 0;
}

int hvmm_mm_init(void)
{
/*
 * MAIR0, MAIR1
 * HMAIR0, HMAIR1
 * HTCR
 * HTCTLR
 * HTTBR
 * HTCTLR
 */
    uint32_t mair, htcr, hsctlr, hcr;
    uint64_t httbr;
    uart_print("[mm] mm_init: enter\n\r");

    vmm_init();
    _hmm_init();

    /* MAIR/HMAIR */
    uart_print(" --- MAIR ----\n\r");
    mair = read_mair0();
    uart_print("mair0:");
    uart_print_hex32(mair);
    uart_print("\n\r");
    mair = read_mair1();
    uart_print("mair1:");
    uart_print_hex32(mair);
    uart_print("\n\r");
    mair = read_hmair0();
    uart_print("hmair0:");
    uart_print_hex32(mair);
    uart_print("\n\r");
    mair = read_hmair1();
    uart_print("hmair1:");
    uart_print_hex32(mair);
    uart_print("\n\r");

    write_mair0(INITIAL_MAIR0VAL);
    write_mair1(INITIAL_MAIR1VAL);
    write_hmair0(INITIAL_MAIR0VAL);
    write_hmair1(INITIAL_MAIR1VAL);

    mair = read_mair0();
    uart_print("mair0:");
    uart_print_hex32(mair);
    uart_print("\n\r");
    mair = read_mair1();
    uart_print("mair1:");
    uart_print_hex32(mair);
    uart_print("\n\r");
    mair = read_hmair0();
    uart_print("hmair0:");
    uart_print_hex32(mair);
    uart_print("\n\r");
    mair = read_hmair1();
    uart_print("hmair1:");
    uart_print_hex32(mair);
    uart_print("\n\r");

    /* HTCR */
    uart_print(" --- HTCR ----\n\r");
    htcr = read_htcr();
    uart_print("htcr:");
    uart_print_hex32(htcr);
    uart_print("\n\r");
    write_htcr(0x80002500);
    htcr = read_htcr();
    uart_print("htcr:");
    uart_print_hex32(htcr);
    uart_print("\n\r");

    /* HSCTLR */
    /* i-Cache and Alignment Checking Enabled */
    /* MMU, D-cache, Write-implies-XN, Low-latency IRQs Disabled */
    hsctlr = read_hsctlr();
    uart_print("hsctlr:");
    uart_print_hex32(hsctlr);
    uart_print("\n\r");
    hsctlr = HSCTLR_BASE | SCTLR_A;
    write_hsctlr(hsctlr);
    hsctlr = read_hsctlr();
    uart_print("hsctlr:");
    uart_print_hex32(hsctlr);
    uart_print("\n\r");


    /* HCR */
    hcr = read_hcr();
    uart_print("hcr:");
    uart_print_hex32(hcr);
    uart_print("\n\r");

    /* HTCR */
    /*
     * Shareability - SH0[13:12] = 0 - Not shared
     * Outer Cacheability - ORGN0[11:10] = 11b -
     *                          Write Back no Write Allocate Cacheable
     * Inner Cacheability - IRGN0[9:8] = 11b - Same
     * T0SZ[2:0] = 0 - 2^32 Input Address
     */
    /* Untested code commented */
/*
    htcr = read_htcr();
    uart_print("htcr:");
    uart_print_hex32(htcr);
    uart_print("\n\r");
    htcr &= ~HTCR_SH0_MASK;
    htcr |= (0x0 << HTCR_SH0_SHIFT) & HTCR_SH0_MASK;
    htcr &= ~HTCR_ORGN0_MASK;
    htcr |= (0x3 << HTCR_ORGN0_SHIFT) & HTCR_ORGN0_MASK;
    htcr &= ~VTCR_IRGN0_MASK;
    htcr |= (0x3 << HTCR_IRGN0_SHIFT) & HTCR_IRGN0_MASK;
    htcr &= ~VTCR_T0SZ_MASK;
    htcr |= (0x0 << HTCR_T0SZ_SHIFT) & HTCR_T0SZ_MASK;
    write_htcr(htcr);
    htcr = read_htcr();
    uart_print("htcr:");
    uart_print_hex32(htcr);
    uart_print("\n\r");
*/

    /* HTTBR = &__hmm_pgtable */
    httbr = read_httbr();
    uart_print("httbr:");
    uart_print_hex64(httbr);
    uart_print("\n\r");
    httbr &= 0xFFFFFFFF00000000ULL;
    httbr |= (uint32_t) &_hmm_pgtable;
    httbr &= HTTBR_BADDR_MASK;
    uart_print("writing httbr:");
    uart_print_hex64(httbr);
    uart_print("\n\r");
    write_httbr(httbr);
    httbr = read_httbr();
    uart_print("read back httbr:");
    uart_print_hex64(httbr);
    uart_print("\n\r");

    /* Enable PL2 Stage 1 MMU */

    hsctlr = read_hsctlr();
    uart_print("hsctlr:");
    uart_print_hex32(hsctlr);
    uart_print("\n\r");

    /* HSCTLR Enable MMU and D-cache */
    hsctlr |= (SCTLR_M | SCTLR_C);

    /* Flush PTE writes */
    asm("dsb");

    write_hsctlr(hsctlr);

    /* Flush iCache */
    asm("isb");

    hsctlr = read_hsctlr();
    uart_print("hsctlr:");
    uart_print_hex32(hsctlr);
    uart_print("\n\r");

    hmm_heap_init();

    uart_print("[mm] mm_init: exit\n\r");

    return HVMM_STATUS_SUCCESS;
}

/**
 * @brief Flush TLB
 *
 * Invalidate entire unified TLB.
 *
 * @return void
 */
void hmm_flushTLB(void)
{
    /* Invalidate entire unified TLB */
    invalidate_unified_tlb(0);
    asm volatile("dsb");
    asm volatile("isb");
}

/**
 * @brief Returns the level 3 table entry.
 *
 * Returns the level 3 translation table descriptor by matching the virtual
 * address and number of pages. If delivered virtual address is over the max
 * memory size, returns zero value.
 *
 * @param virt Virtual address.
 * @param npages Number of pages.
 * @return The level 3 table entry.
 */
union lpaed *hmm_get_l3_table_entry(unsigned long virt, unsigned long npages)
{
    int l2_index = (virt >> L2_SHIFT) & L2_ENTRY_MASK;
    int l3_index = (virt >> L3_SHIFT) & L3_ENTRY_MASK;
    int maxsize = ((HMM_L2_PTE_NUM * HMM_L3_PTE_NUM) - \
            ((l2_index + 1) * (l3_index + 1)) + 1);
    if (maxsize < npages) {
        printh("%s[%d] : Map size \"pages\" is exceeded memory size\n",
                __func__, __LINE__);
        if (maxsize > 0)
            printh("%s[%d] : Available pages are %d\n", maxsize);
        else
            printh("%s[%d] : Do not have available pages for map\n");
        return 0;
    }
    return &_hmm_pgtable_l3[l2_index][l3_index];
}

void hmm_umap(unsigned long virt, unsigned long npages)
{
    int  i;
    union lpaed *map_table_p = hmm_get_l3_table_entry(virt, npages);
    for (i = 0; i < npages; i++)
        lpaed_stage1_disable_l3_table(&map_table_p[i]);
    hmm_flushTLB();
}

void hmm_map(unsigned long phys, unsigned long virt, unsigned long npages)
{
    int i;
    union lpaed *map_table_p = hmm_get_l3_table_entry(virt, npages);
    for (i = 0; i < npages; i++)
        lpaed_stage1_conf_l3_table(&map_table_p[i], (uint64_t)phys, 1);
    hmm_flushTLB();
}

/**
 * @brief General-purpose sbrk, basic memory management system calls.
 *
 * General-purpose sbrk, basic memory management system calls.
 * If there was no heap memory space, returns the value -1.
 *
 * @param  incr Size of memory wanted.
 * @return Pointer of allocated heap memory.
 */
void *hmm_sbrk(unsigned int incr)
{
    unsigned int required_addr;
    unsigned int virt;
    unsigned int required_pages = 0;

    mm_prev_break = mm_break;
    virt = mm_break;
    mm_break += incr;
    if (mm_break > last_valid_address) {
        required_addr = mm_break - last_valid_address;
        for (; required_addr > 0x0; required_addr -= 0x1000) {
            if (last_valid_address + 0x1000 > HEAP_END_ADDR) {
                printh("%s[%d] required address is exceeded heap memory size\n",
                        __func__, __LINE__);
                return (void *)-1;
            }
            last_valid_address += 0x1000;
            required_pages++;
        }
        hmm_map(virt, virt, required_pages);
    }
    return (void *)mm_prev_break;
}

void hmm_free(void *ap)
{
    union header *bp, *p;
    bp = (union header *)ap - 1; /* point to block header */
    for (p = freep; !(bp > p && bp  < p->s.ptr); p = p->s.ptr) {
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
            break; /* freed block at start or end of arena */
    }
    if (bp + bp->s.size == p->s.ptr) { /* join to upper nbr */
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else
        bp->s.ptr = p->s.ptr;
    if (p + p->s.size == bp) {      /* join to lower nbr */
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else
        p->s.ptr = bp;
    freep = p;
}

/**
 * @brief Obtains the storage from the heap memory.
 *
 * If wanted size of block is not bigger then NALLOC value, this function
 * obtains minimally size of memory.(1024 Bytes)
 * Makes free memory area by calling free function after obtain the memory.
 *
 * @param nu Number of units, allocate size.
 * @return Pointer of free list block header & memory area.
 *         The size of this memory is bigger than 1KB.
 */
static union header *morecore(unsigned int nu)
{
    char *cp;
    union header *up;
    if (nu < NALLOC)
        nu = NALLOC;
    cp = hmm_sbrk(nu * sizeof(union header));
    if (cp == (char *) -1) /* no space at all */
        return 0;
    up = (union header *)cp;
    up->s.size = nu;
    hmm_free((void *)(up+1));
    return freep;
}

void *hmm_malloc(unsigned long size)
{
    union header *p, *prevp;
    unsigned int nunits;
    nunits = (size + sizeof(union header) - 1)/sizeof(union header) + 1;
    if (nunits < 2)
        return 0;
    prevp = freep;
    if ((prevp) == 0) { /* no free list yet */
        freep_base.s.ptr = freep = prevp = &freep_base;
        freep_base.s.size = 0;
    }
    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) { /* big enough */
            if (p->s.size == nunits) /* exactly */
                prevp->s.ptr = p->s.ptr;
            else {                    /* allocate tail end */
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void *)(p+1);
        }
        if (p == freep) {  /* wrapped around free list */
            p = morecore(nunits);
            if (p == 0)
                return 0;  /* none avaliable memory left */
        }
    }
}

