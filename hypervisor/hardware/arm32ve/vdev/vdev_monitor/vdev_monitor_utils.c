#include <monitor.h>
#include <interrupt.h>
#include <asm_io.h>
#include <asm-arm_inline.h>
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

/*
 * Guest's VA to PA Monitoring
 */
uint64_t va_to_pa(vcpuid_t vmid, uint32_t va, uint32_t ttbr_num)
{
    uint32_t linux_guest_ttbr, l1_des, l2_des;

    /* Not MMU mode */
//    return va;

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

void invalidate_icache_all(void)
{
    /*
       Invalidate all instruction caches to PoU.
       Also flushes branch target cache.
    */
    asm volatile ("mcr p15, 0, %0, c7, c5, 0" : : "r" (0));
    /* Invalidate entire branch predictor array */
    asm volatile ("mcr p15, 0, %0, c7, c5, 6" : : "r" (0));
    /* Full system DSB - make sure that the invalidation is complete
     */
    CP15DSB;
    /* Full system ISB - make sure the instruction stream sees it */
    CP15ISB;
}
#define ARMV7_DCACHE_INVAL_ALL              1
#define ARMV7_DCACHE_CLEAN_INVAL_ALL        2
#define ARMV7_DCACHE_INVAL_RANGE            3
#define ARMV7_DCACHE_CLEAN_INVAL_RANGE      4
#define ARMV7_CSSELR_IND_DATA_UNIFIED       0
#define ARMV7_CSSELR_IND_INSTRUCTION        1
#define ARMV7_CLIDR_CTYPE_NO_CACHE          0
#define ARMV7_CLIDR_CTYPE_INSTRUCTION_ONLY  1
#define ARMV7_CLIDR_CTYPE_DATA_ONLY         2
#define ARMV7_CLIDR_CTYPE_INSTRUCTION_DATA  3
#define ARMV7_CLIDR_CTYPE_UNIFIED           4
/* CCSIDR */
#define CCSIDR_LINE_SIZE_OFFSET     0
#define CCSIDR_LINE_SIZE_MASK       0x7
#define CCSIDR_ASSOCIATIVITY_OFFSET 3
#define CCSIDR_ASSOCIATIVITY_MASK   (0x3FF << 3)
#define CCSIDR_NUM_SETS_OFFSET      13
#define CCSIDR_NUM_SETS_MASK        (0x7FFF << 13)

static inline int log_2_n_round_up(uint32_t n)
{
    int log2n = -1;
    uint32_t temp = n;

    while (temp) {
        log2n++;
        temp >>= 1;
    }

    if (n & (n - 1))
        return log2n + 1; /* not power of 2 - round up
                           */
    else
        return log2n; /* power of 2 */
}

static inline int log_2_n_round_down(int n)
{
    int log2n = -1;
    uint32_t temp = n;

    while (temp) {
        log2n++;
        temp >>= 1;
    }

    return log2n;
}


/*
 * Write the level and type you want to Cache Size Selection Register(CSSELR)
 * to get size details from Current Cache Size ID Register(CCSIDR)
 */
static void set_csselr(uint32_t level, uint32_t type)
{   uint32_t csselr = level << 1 | type;

    /* Write to Cache Size Selection Register(CSSELR) */
    asm volatile ("mcr p15, 2, %0, c0, c0, 0" : : "r" (csselr));
}

static uint32_t get_ccsidr(void)
{
    uint32_t ccsidr;

    /* Read current CP15 Cache Size ID Register */
    asm volatile ("mrc p15, 1, %0, c0, c0, 0" : "=r" (ccsidr));
    return ccsidr;
}

static uint32_t get_clidr(void)
{
    uint32_t clidr;

    /* Read current CP15 Cache Level ID Register */
    asm volatile ("mrc p15,1,%0,c0,c0,1" : "=r" (clidr));
    return clidr;
}

static void v7_inval_dcache_level_setway(uint32_t level, uint32_t num_sets,
        uint32_t num_ways, uint32_t way_shift, uint32_t log2_line_len)
{
    int way, set, setway;
    for (way = num_ways - 1; way >= 0 ; way--) {
        for (set = num_sets - 1; set >= 0; set--) {
            setway = (level << 1) | (set << log2_line_len) |
                (way << way_shift);
            /* Invalidate data/unified cache line by set/way */
            asm volatile (" mcr p15, 0, %0, c7, c6, 2" : : "r"(setway));
        }
    }
    CP15DSB;
}

static void v7_dcache_clean_inval_range(uint32_t start,
        uint32_t stop, uint32_t line_len)
{
    uint32_t mva;

    /* Align start to cache line boundary */
    start &= ~(line_len - 1);
    for (mva = start; mva < stop; mva = mva + line_len) {
        /* DCCIMVAC - Clean & Invalidate data cache by MVA
         * to PoC */
        asm volatile ("mcr p15, 0, %0, c7, c14, 1" : : "r"
                (mva));
    }
}

static void v7_dcache_inval_range(uint32_t start, uint32_t stop,
        uint32_t line_len)
{
    uint32_t mva;

    /*  
     *  If start address is not aligned to cache-line do not
     *  invalidate the first cache-line
     */
    if (start & (line_len - 1)) {
        printh("ERROR: %s - start address is not aligned - 0x%08x\n", __func__, start);
        /* move to next cache line */
        start = (start + line_len - 1) & ~(line_len - 1);
    }

    /*  
     * If stop address is not aligned to cache-line do not
     * invalidate the last cache-line
     */
    if (stop & (line_len - 1)) {
        printh("ERROR: %s - stop address is not aligned - 0x%08x\n", __func__, stop);
        /* align to the beginning of this cache line
         */
        stop &= ~(line_len - 1);
    }

    for (mva = start; mva < stop; mva = mva + line_len) {
        /* DCIMVAC - Invalidate data cache by MVA to PoC
         */
        asm volatile ("mcr p15, 0, %0, c7, c6, 1" : : "r" (mva));
    }
}

static void v7_dcache_maint_range(uint32_t start, uint32_t stop, uint32_t range_op)
{
    uint32_t line_len, ccsidr;

    ccsidr = get_ccsidr();
    line_len = ((ccsidr & CCSIDR_LINE_SIZE_MASK) >>
            CCSIDR_LINE_SIZE_OFFSET) + 2;
    /* Converting from words to bytes */
    line_len += 2;
    /* converting from log2(linelen) to linelen */
    line_len = 1 << line_len;

    switch (range_op) {
    case ARMV7_DCACHE_CLEAN_INVAL_RANGE:
        v7_dcache_clean_inval_range(start, stop, line_len);
        break;
        case
            ARMV7_DCACHE_INVAL_RANGE:
            v7_dcache_inval_range(start, stop, line_len);
        break;
    }

    /* DSB to make sure the operation is complete */
    CP15DSB;
}



static void v7_clean_inval_dcache_level_setway(uint32_t level,
        uint32_t num_sets, uint32_t num_ways, uint32_t way_shift,
        uint32_t log2_line_len)
{
    int way, set, setway;

    for (way = num_ways - 1; way >= 0 ; way--) {
        for (set = num_sets - 1; set >= 0; set--) {
            setway = (level << 1) | (set << log2_line_len) |
                (way << way_shift);
            /*
             * Clean & Invalidate data/unified cache line by set/way
             */
            asm volatile (" mcr p15, 0, %0, c7, c14, 2" : : "r" (setway));
        }
    }
    /* DSB to make sure the operation is complete */
    CP15DSB;
}

static void v7_maint_dcache_level_setway(uint32_t level, uint32_t operation)
{
    uint32_t ccsidr;
    uint32_t num_sets, num_ways, log2_line_len, log2_num_ways;
    uint32_t way_shift;

    set_csselr(level, ARMV7_CSSELR_IND_DATA_UNIFIED);

    ccsidr = get_ccsidr();

    log2_line_len = ((ccsidr & CCSIDR_LINE_SIZE_MASK) >>
            CCSIDR_LINE_SIZE_OFFSET) + 2;
    /* Converting from words to bytes */
    log2_line_len += 2;

    num_ways  = ((ccsidr & CCSIDR_ASSOCIATIVITY_MASK) >>
            CCSIDR_ASSOCIATIVITY_OFFSET) + 1;
    num_sets  = ((ccsidr & CCSIDR_NUM_SETS_MASK) >>
            CCSIDR_NUM_SETS_OFFSET) + 1;

    log2_num_ways = log_2_n_round_up(num_ways);

    way_shift = (32 - log2_num_ways);
    if (operation == ARMV7_DCACHE_INVAL_ALL) {
        v7_inval_dcache_level_setway(level, num_sets, num_ways, way_shift,
                log2_line_len);
    } else if (operation == ARMV7_DCACHE_CLEAN_INVAL_ALL) {
                v7_clean_inval_dcache_level_setway(level, num_sets, num_ways,
                        way_shift, log2_line_len);
            }
}

static void v7_maint_dcache_all(uint32_t operation)
{
    uint32_t level, cache_type, level_start_bit = 0;

    uint32_t clidr = get_clidr();

    for (level = 0; level < 7; level++) {
        cache_type = (clidr >> level_start_bit) & 0x7;
        if ((cache_type == ARMV7_CLIDR_CTYPE_DATA_ONLY) ||
            (cache_type == ARMV7_CLIDR_CTYPE_INSTRUCTION_DATA) ||
            (cache_type == ARMV7_CLIDR_CTYPE_UNIFIED))
            v7_maint_dcache_level_setway(level, operation);
        level_start_bit += 3;
    }
}

/*
 * Invalidates range in all levels of D-cache/unified cache used:
 * Affects the range [start, stop - 1]
 */
void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
    v7_dcache_maint_range(start, stop, ARMV7_DCACHE_INVAL_RANGE);
}

/*
 * Flush range(clean & invalidate) from all levels of D-cache/unified
 * cache used:
 * Affects the range [start, stop - 1]
 */
void flush_dcache_range(unsigned long start, unsigned long stop)
{
    v7_dcache_maint_range(start, stop, ARMV7_DCACHE_CLEAN_INVAL_RANGE);
}


void invalidate_dcache_all(void)
{
    v7_maint_dcache_all(ARMV7_DCACHE_INVAL_ALL);
}

/*
 * Flush range from all levels of d-cache/unified-cache used:
 * Affects the range [start, start + size - 1]
 */

void flush_cache(unsigned long start, unsigned long size)
{
    flush_dcache_range(start, start + size);
}


/*
 * Performs a clean & invalidation of the entire data cache at all levels
 */
void flush_dcache_all(void)
{
    v7_maint_dcache_all(ARMV7_DCACHE_CLEAN_INVAL_ALL);
}

