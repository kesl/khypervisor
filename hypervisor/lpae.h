#ifndef __LPAE_H__
#define __LPAE_H__
#include <arch_types.h>

#define LPAE_PAGE_SHIFT    12
#define LPAE_PAGE_SIZE      (1<<LPAE_PAGE_SHIFT)
#define LPAE_PAGE_MASK      (0xFFF)

#define LPAE_BLOCK_L2_SHIFT 21
#define LPAE_BLOCK_L2_SIZE  (1<<LPAE_BLOCK_L2_SHIFT)
#define LPAE_BLOCK_L2_MASK  (0x1FFFFF)

/**
 * \defgroup Attribute_Indexes
 *
 * These are valid in the AttrIndx[2:0] field of an LPAE stage 1 page<br>
 * table entry. They are indexes into the bytes of the MAIR*<br>
 * registers, as defined above.
 * @{
 */
#define ATTR_IDX_UNCACHED      0x0
#define ATTR_IDX_BUFFERABLE    0x1
#define ATTR_IDX_WRITETHROUGH  0x2
#define ATTR_IDX_WRITEBACK     0x3
#define ATTR_IDX_DEV_SHARED    0x4
#define ATTR_IDX_WRITEALLOC    0x7
#define ATTR_IDX_DEV_NONSHARED DEV_SHARED
#define ATTR_IDX_DEV_WC        BUFFERABLE
#define ATTR_IDX_DEV_CACHED    WRITEBACK
/** @}*/
/******************************************************************************
 * ARMv7-A LPAE pagetables:3-level trie, mapping 40-bit input to
 * 40-bit output addresses.  Tables at all levels have 512 64-bit entries
 * (i.e. are 4Kb long).
 *
 * The bit-shuffling that has the permission bits in branch nodes in a
 * different place from those in leaf nodes seems to be to allow linear
 * pagetable tricks.  If we're not doing that then the set of permission
 * bits that's not in use in a given node type can be used as
 * extra software-defined bits. */

/**
 * @brief LPAE page table descriptor
 *
 * - Descriptoin<br>
 *   - These are used in all kinds of entry<br>
 *     - valid - Valid mapping<br>
 *     - Table - == 1 - Table Entry, == 0 block or page entry<br>
 *   - These ten bits are only used in Block entries<br>
 *     and are ignored in Table Entries<br>
 *     - ai - Attributes Index<br>
 *     - ns - Non-Secure<br>
 *     - User - User-visible (PL0)<br>
 *     - ro - Read-Only<br>
 *     - sh - Shareability<br>
 *     - af - Access Flag<br>
 *     - ng - Not-Global<br>
 *   - The base address must be appropriately aligned for Block entries<br>
 *     - base - base address of physical address or other Table entry<br>
 *   - Theses seven bits are only used in Block entries and are ignored<br>
 *     - hint - Contiguous hint, In a block of 16 contiguous entries<br>
 *     - pxn - Privileged-eXecute-Never<br>
 *     - xn - eXecute-Never<br>
 *     - avail - Ignored by Hardware<br>
 *   - These 5 bits are only used in Table entries<br>
 *     and are ignored in Block entries<br>
 *     - pxnt - Privileged-eXeucte-Never<br>
 *     - xnt - eXecute-Never<br>
 *     - apt - Access Permissions<br>
 *     - nst - Not-Secure<br>
 */
struct lpae_pt {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;     /**< Valid mapping */
    unsigned long table:1;     /**< == 1 in 4k map entries too */

    /* These ten bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long ai:3;        /**< Attribute Index */
    unsigned long ns:1;        /**< Not-Secure */
    unsigned long user:1;      /**< User-visible */
    unsigned long ro:1;        /**< Read-Only */
    unsigned long sh:2;        /**< Shareability */
    unsigned long af:1;        /**< Access Flag */
    unsigned long ng:1;        /**< Not-Global */

    /* The base address must be appropriately aligned for Block entries */
    unsigned long base:28;     /**< Base address of block or next table */
    unsigned long sbz:12;      /**< Must be zero */

    /* These seven bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long hint:1;      /**< In a block of 16 contiguous entries */
    unsigned long pxn:1;       /**< Privileged-XN */
    unsigned long xn:1;        /**< eXecute-Never */
    unsigned long avail:4;     /**< Ignored by hardware */

    /* These 5 bits are only used in Table entries and are ignored in
     * Block entries */
    unsigned long pxnt:1;      /**< Privileged-XN */
    unsigned long xnt:1;       /**< eXecute-Never */
    unsigned long apt:2;       /**< Access Permissions */
    unsigned long nst:1;       /**<  Not-Secure */
} __attribute__((__packed__));

/**
 * @brief LPAE p2m table descriptor
 *
 * The p2m tables have almost the same layout, but some of the permission<br>
 * and cache-control bits are laid out differently (or missing)<br>
 * - Description<br>
 *   - These are used in all kinds of entry<br>
 *     - valid - Valid mapping<br>
 *     - table - == 1 Table Entry, == 0 block or page Entry<br>
 *   - These ten bits are only used in Block entries<br>
 *     and are ignored in Table Entries<br>
 *     - mattr - Memory Attributes<br>
 *     - read - Read Access<br>
 *     - write - Write Access<br>
 *     - sh - Shareability<br>
 *     - af - Access Flag<br>
 *   - The base address must be appropriately aligned for Block entries<br>
 *     - base - Bass Address of block or next table<br>
 *   - These six bits are only used in Block entries<br>
 *     and are ignored in Table entries<br>
 *     - hint - Contiguous Hint, In a block of 16 contiguous entries<br>
 *     - xn - eXecute-Never<br>
 *     - avail - Ignored by Hardware<br>
 */
struct lpae_p2m {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;     /**< Valid mapping */
    unsigned long table:1;     /**< == 1 in 4k map entries too */

    /* These ten bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long mattr:4;     /**< Memory Attributes */
    unsigned long read:1;      /**< Read access */
    unsigned long write:1;     /**< Write access */
    unsigned long sh:2;        /**< Shareability */
    unsigned long af:1;        /**< Access Flag */
    unsigned long sbz4:1;

    /* The base address must be appropriately aligned for Block entries */
    unsigned long base:28;     /**< Base address of block or next table */
    unsigned long sbz3:12;

    /* These seven bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long hint:1;      /**< In a block of 16 contiguous entries */
    unsigned long sbz2:1;
    unsigned long xn:1;        /**< eXecute-Never */
    unsigned long avail:4;     /**< Ignored by hardware */

    unsigned long sbz1:5;
} __attribute__((__packed__));


/**
 * @brief Walk entry
 *
 * Walk is the common bits of p2m and pt entries which are needed to<br>
 * simply walk the table (e.g. for debug).<br>
 * - Description<br>
 *   - valid - Valid mapping<br>
 *   - table - == 1 in 4k map entries too<br>
 *   - pad2 - <br>
 *   - base - Base address of block or next table<br>
 *   - pad1 - <br>
 */
struct lpae_walk {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;     /**< Valid mapping */
    unsigned long table:1;     /**< == 1 in 4k map entries too */

    unsigned long pad2:10;

    /* The base address must be appropriately aligned for Block entries */
    unsigned long base:28;     /**< Base address of block or next table */

    unsigned long pad1:24;
} __attribute__((__packed__));

/**
 * @brief Lpead's union
 */
union lpaed {
    uint64_t bits;
    struct lpae_pt pt;
    struct lpae_p2m  p2m;
    struct lpae_walk walk;
};

/**
 * @brief Lpaed stage2 memory attribute configuration enum
 */
enum lpaed_stage2_memattr {
    LPAED_STAGE2_MEMATTR_SO = 0x0,   /**< Strongly Ordered */
    LPAED_STAGE2_MEMATTR_DM = 0x1,   /**< Device memory */
    LPAED_STAGE2_MEMATTR_NORMAL_ONC = 0x4,  /**< Outer Non-cacheable */
    LPAED_STAGE2_MEMATTR_NORMAL_OWT = 0x8,  /**< Outer Write-through */
    LPAED_STAGE2_MEMATTR_NORMAL_OWB = 0xC,  /**< Outer Write-back */
    LPAED_STAGE2_MEMATTR_NORMAL_INC = 0x1,  /**< Inner Non-cacheable */
    LPAED_STAGE2_MEMATTR_NORMAL_IWT = 0x2,  /**< Inner Write-through */
    LPAED_STAGE2_MEMATTR_NORMAL_IWB = 0x3,  /**< Inner Write-back */
};

/**
 * @brief Level 1 Block, 1GB, entry in LPAE Descriptor format for the given physical addres
 *
 * Generate entry of LPAE Descriptor - Level1 Block, 1GB<br>
 * - Setting<br>
 *   - Valid =1, table = 0, ai = attr_idx<br>
 *   - ns = 1, user =1, ro = 0, sh = 2<br>
 *   - af = 1, ng = 1, hint = 0, pxn = 0, xn = 0<br>
 * @param uint64_t pa block's physical address
 * @param uint8_t attr_idx attribute index for memory this descriptor
 * @return lpaed_t generated entry of LPAE Descriptor
 */
union lpaed hvmm_mm_lpaed_l1_block(uint64_t pa, uint8_t attr_idx);
/**
 * @brief Level 2 Block, 2MB, entry in LPAE Descriptor format for the given physical address
 *
 * Generate entry of LPAE Descriptor - Level2 Block, 2MB<br>
 * - Setting<br>
 *   - Valid = 1, table = 0, mattr = mattr & 0x0F<br>
 *   - read = 1, write =1, sh = 0, af = 1, hint = 0<br>
 *   - xn = 0<br>
 * @param uint64_t pa block's physical address
 * @param lpaed_stage2_memattr_t mattr
 * @return lpaed_t generated entry of LPAE Descriptor
 */
union lpaed hvmm_mm_lpaed_l2_block(uint64_t pa,
                enum lpaed_stage2_memattr mattr);
/**
 * @brief Level 1, 1GB, each entry refer level2 page table
 *
 * Generate entry of LPAE Descriptor - Level 1 table entry, 1GB<br>
 * - Setting<br>
 *   - valid = 1, table = 1, pxnt = 0, xnt = 0, apt = 0, nst = 1<br>
 * @param uint64_t pa level2 page table's physical address
 * @return lpead_t generated entry of LPAE Descriptor
 */
union lpaed hvmm_mm_lpaed_l1_table(uint64_t pa);
/**
 * @brief Level 2, 2MB, each entry refer level3 page table
 *
 * Generate entry of LPAE Descriptor - Level 2 table entry, 2MB<br>
 * - Setting<br>
 *   - Valid = 1, table = 1, pxnt = 0, xnt = 0, apt =0, nst =1<br>
 * @param uint64_t pa level3 page table's physical address
 * @return lpead_t generated entry of LPAE Descriptor
 */
union lpaed hvmm_mm_lpaed_l2_table(uint64_t pa);
/**
 * @brief Level 3, each entry refer 4KB physical address
 *
 * Generate entry of LPAE Descriptor - Level 3 table entry, 4KB<br>
 * - Setting<br>
 *   - Valid = valid, table = 1, ai = attr_idx, ns = 1, user = 1<br>
 *   - ro = 0, sh = 2, af =1, ng =1, hint = 0, pxn = 0, xn =0<br>
 * @param uint64_t pa 4KB physical address
 * @param uint8_t attr_idx memory attribute Index
 * @param uint8_t valid validation of table entry
 * @return lpead_t generated entry of LPAE Descriptor
 */
union lpaed hvmm_mm_lpaed_l3_table(uint64_t pa, uint8_t attr_idx,
                uint8_t valid);
/**
 * @brief Configure stage1 level3 table entry
 *
 * configure table's validate & page base address<br>
 * - valid = valid ? 1 : 0
 * @param lpaed_t *ttbl3 level3 translation table entry pointer
 * @param uint64_t baddr delivered base address
 * @param uint8_5 valid validate
 * @return void
 */
void lpaed_stage1_conf_l3_table(union lpaed *ttbl3, uint64_t baddr,
                uint8_t valid);
/**
 * @brief Disable stage1 level3 table entry
 *
 * make Invalidate table entry<br>
 * - valid = 0
 * @param lpaed_t *ttbl3 target table entry pointer
 * @return void
 */
void lpaed_stage1_disable_l3_table(union lpaed *ttbl2);
/**
 * @brief Mapping stage2 lpaed to physical address & memory attrigute
 *
 * mapping page table entry to pa, mattr<br>
 * - Setting<br>
 *   - valid = 1, table = 1, mattr = mattr & 0x0F, read = 1, write 1<br>
 *   - sh = 0, af = 1, hint = 0, xn = 0
 * @param lpaed_t *pte target page table entry
 * @param uint64_t pa physical address
 * @param lpaed_stage2_memattr_t mattr memory entry
 * @return void
 */
void lpaed_stage2_map_page(union lpaed *pte, uint64_t pa,
        enum lpaed_stage2_memattr mattr);
/**
 * @brief Configure stage2 level 1 table entry's valid & table bit & set baddr
 *
 * - Setting<br>
 *   - valid = valid ? 1: 0, table = valid ? 1 : 0<br>
 *   - bass address = baddr
 * @param lpaed_t *ttbl1 stage2 level1 table's translation table entry
 * @param uint64_t baddr base address
 * @param uint8_5 valid valid
 * @return void
 */
void lpaed_stage2_conf_l1_table(union lpaed *ttbl1, uint64_t baddr,
        uint8_t valid);
/**
 * @brief Configure stage2 level 2 table entry's valid & table bit & set baddr
 *
 * - Setting<br>
 *   - valid = valid ? 1: 0, table = valid ? 1 : 0<br>
 *   - bass address = baddr
 * @param lpaed_t *ttbl2 stage2 level2 table's translation table entry
 * @param uint64_t baddr base address
 * @param uint8_5 valid valid
 * @return void
 */
void lpaed_stage2_conf_l2_table(union lpaed *ttbl2, uint64_t baddr,
        uint8_t valid);
/**
 * @brief Enable stage2 level2 table entry
 *
 * - Setting<br>
 *   - valid = 1, table =1
 * @param lpaed_t *ttbl2 target level2 translation table entry
 * @return void
 */
void lpaed_stage2_enable_l2_table(union lpaed *ttbl2);
/**
 * @brief Disable stage2 level2 table entry
 *
 * - Setting<br>
 *   - valid = 0
 * @param lpaed_t *ttbl2 target level2 translation table entry
 * @return void
 */
void lpaed_stage2_disable_l2_table(union lpaed *ttbl2);

#endif
