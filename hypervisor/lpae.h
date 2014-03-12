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
 * <pre>
 * These are valid in the AttrIndx[2:0] field of an LPAE stage 1 page
 * table entry. They are indexes into the bytes of the MAIR*
 * registers, as defined above.
 * </pre>
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

/**
 * @file lpae.h
 *
 * <pre>
 * ARMv7-A LPAE pagetables:3-level trie, mapping 40-bit input to
 * 40-bit output addresses.  Tables at all levels have 512 64-bit entries
 * (i.e. are 4Kb long).
 * The bit-shuffling that has the permission bits in branch nodes in a
 * different place from those in leaf nodes seems to be to allow linear
 * pagetable tricks.  If we're not doing that then the set of permission
 * bits that's not in use in a given node type can be used as
 * extra software-defined bits.
 * </pre>
 */

/**
 * @brief LPAE page table descriptor
 *
 * - Descriptoin
 *   - These are used in all kinds of entry
 *     - valid - Valid mapping
 *     - Table - == 1 - Table Entry, == 0 block or page entry
 *   - These ten bits are only used in Block entries
 *     and are ignored in Table Entries
 *     - ai - Attributes Index
 *     - ns - Non-Secure
 *     - User - User-visible (PL0)
 *     - ro - Read-Only
 *     - sh - Shareability
 *     - af - Access Flag
 *     - ng - Not-Global
 *   - The base address must be appropriately aligned for Block entries
 *     - base - base address of physical address or other Table entry
 *   - Theses seven bits are only used in Block entries and are ignored
 *     - hint - Contiguous hint, In a block of 16 contiguous entries
 *     - pxn - Privileged-eXecute-Never
 *     - xn - eXecute-Never
 *     - avail - Ignored by Hardware
 *   - These 5 bits are only used in Table entries
 *     and are ignored in Block entries
 *     - pxnt - Privileged-eXeucte-Never
 *     - xnt - eXecute-Never
 *     - apt - Access Permissions
 *     - nst - Not-Secure
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
 * @brief LPAE table descriptor for stage-2 translation
 *
 * This descriptor has almost the same layout to lpae_pt,<br>
 * But it is stage-2 translation descriptor
 * - Description
 *   - These are used in all kinds of entry
 *     - valid - Valid mapping
 *     - table - == 1 Table Entry, == 0 block or page Entry
 *   - These ten bits are only used in Block entries
 *     and are ignored in Table Entries
 *     - mattr - Memory Attributes
 *     - read - Read Access
 *     - write - Write Access
 *     - sh - Shareability
 *     - af - Access Flag
 *   - The base address must be appropriately aligned for Block entries
 *     - base - Bass Address of block or next table
 *   - These six bits are only used in Block entries
 *     and are ignored in Table entries
 *     - hint - Contiguous Hint, In a block of 16 contiguous entries
 *     - xn - eXecute-Never
 *     - avail - Ignored by Hardware
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
    unsigned long base:28;     /* Base address of block or next table */
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
 * simply walk the table (e.g. for debug).
 * - Description
 *   - valid - Valid mapping
 *   - table - == 1 in 4k map entries too
 *   - pad2 -
 *   - base - Base address of block or next table
 *   - pad1 -
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
 * @brief Level 1 block, 1GB, entry in LPAE descriptor format
 * for the given physical addres
 *
 * Generate a new level 1 block LPAE descriptor which has 1GB unit size<br>
 * Return the descriptor After generating
 * - Initial bit state
 *   - Valid = 1, table = 0, ai = attr_idx
 *   - ns = 1, user =1, ro = 0, sh = 2
 *   - af = 1, ng = 1, hint = 0, pxn = 0, xn = 0
 *   - physical address = pa
 * @param  pa Block's physical address
 * @param  attr_idx Attribute index for memory this descriptor
 * @return  Generated level1 block LPAE Descriptor
 */
union lpaed hvmm_mm_lpaed_l1_block(uint64_t pa, uint8_t attr_idx);
/**
 * @brief Level 2 block, 2MB, entry in LPAE Descriptor format
 * for the given physical address
 *
 * Generate a new level 2 block LPAE descriptor which has 2MB unit size<br>
 * Return the descriptor after generating
 * - Initial bit state
 *   - Valid = 1, table = 0, mattr = mattr & 0x0F
 *   - read = 1, write =1, sh = 0, af = 1, hint = 0
 *   - xn = 0, physical address = pa
 * @param pa Block's physical address
 * @param mattr Memory attribute index
 * @return Generated level 2 block LPAE Descriptor
 */
union lpaed hvmm_mm_lpaed_l2_block(uint64_t pa,
                enum lpaed_stage2_memattr mattr);
/**
 * @brief Level 1, 1GB, each entry refer level2 page table
 *
 * Generate a new level 1 LPAE descriptor which refers level 2 page table<br>
 * Return the descriptor after generating
 * - Initial bit state
 *   - valid = 1, table = 1, pxnt = 0, xnt = 0, apt = 0, nst = 1
 *   - physical address = pa
 * @param pa Refers level2 page table's physical address
 * @return Generated level 1 page table LPAE Descriptor
 */
union lpaed hvmm_mm_lpaed_l1_table(uint64_t pa);
/**
 * @brief Level 2, 2MB, each entry refer level3 page table
 *
 * Generate a new level 2 LPAE Descriptor which refers level 3 page table<br>
 * Return the descriptor after generating
 * - Initial bit state
 *   - Valid = 1, table = 1, pxnt = 0, xnt = 0, apt =0, nst =1
 *   - physical address = pa
 * @param pa Refers level3 page table's physical address
 * @return Generated level 2 page table LPAE Descriptor
 */
union lpaed hvmm_mm_lpaed_l2_table(uint64_t pa);
/**
 * @brief Level 3, each entry refer 4KB physical address
 *
 * Generate a new level 3 LPAE Descriptor which refers 4KB physical address<br>
 * Return the descriptor after generating
 * - Initial bit state
 *   - Valid = valid, table = 1, ai = attr_idx, ns = 1, user = 1
 *   - ro = 0, sh = 2, af =1, ng =1, hint = 0, pxn = 0, xn =0
 *   - physical address = pa
 * @param pa 4KB physical address
 * @param attr_idx memory Attribute Index
 * @param valid Validation of table descriptor
 * @return Generated level 3 page table LPAE Descriptor
 */
union lpaed hvmm_mm_lpaed_l3_table(uint64_t pa, uint8_t attr_idx,
                uint8_t valid);
/**
 * @brief Configure stage-1 level 3 table entry
 *
 * Configure the table descriptor's validate bit<br>
 * Wrtie the page address
 * - valid = valid ? 1 : 0
 * - physical address = pa
 * @param *ttbl3 Pointer of level3 translation table descriptor
 * @param baddr Target page address
 * @param valid Validate
 * @return void
 */
void lpaed_stage1_conf_l3_table(union lpaed *ttbl3, uint64_t baddr,
                uint8_t valid);
/**
 * @brief Disable stage-1 level 3 table entry
 *
 * Make invalidate table Descriptor
 * - valid = 0
 * @param *ttbl3 Pointer of level 3 table descriptor
 * @return void
 */
void lpaed_stage1_disable_l3_table(union lpaed *ttbl2);
/**
 * @brief Mapping stage-2 level 2 lpae descriptor to physical address
 * and  memory attribute
 *
 * Reconfigure the descriptor to the level 2 page table descriptor<br>
 * And mapping the physical address and memory attribute
 * - State configuration
 *   - valid = 1, table = 1, mattr = mattr & 0x0F, read = 1, write 1
 *   - sh = 0, af = 1, hint = 0, xn = 0 physical address = pa
 * @param *pte Page table descriptor
 * @param pa Physical address
 * @param mattr Memory entry
 * @return void
 */
void lpaed_stage2_map_page(union lpaed *pte, uint64_t pa,
        enum lpaed_stage2_memattr mattr);
/**
 * @brief Configure stage-2 level 1 table entry's valid & table bit
 * and set the base address
 *
 * Configure the descriptor's valid bit and table bit<br>
 * And mapping the physical address
 * - State configuration
 *   - valid = valid ? 1: 0, table = valid ? 1 : 0
 *   - bass address = baddr
 * @param *ttbl1 Stage-2 level1 table's translation table entry
 * @param baddr Base address
 * @param valid Validation
 * @return void
 */
void lpaed_stage2_conf_l1_table(union lpaed *ttbl1, uint64_t baddr,
        uint8_t valid);
/**
 * @brief Configure stage-2 level 2 table entry's valid & table bit
 * and set the base address
 *
 * Configure the descriptor's valid bit and table bit<br>
 * And set the base address
 * - State configuration
 *   - valid = valid ? 1: 0, table = valid ? 1 : 0
 *   - bass address = baddr
 * @param *ttbl2 Stage-2 level2 table's translation table entry
 * @param baddr Base address
 * @param valid Validation
 * @return void
 */
void lpaed_stage2_conf_l2_table(union lpaed *ttbl2, uint64_t baddr,
        uint8_t valid);
/**
 * @brief Enable stage-2 level2 table entry
 *
 * Enabling the level 2 descriptor's valid bit and table bit
 * - State
 *   - valid = 1, table =1
 * @param *ttbl2 Level2 translation table descriptor
 * @return void
 */
void lpaed_stage2_enable_l2_table(union lpaed *ttbl2);
/**
 * @brief Disable stage-2 level2 table entry
 *
 * Disable the descriptor's valid bit
 * - State
 *   - valid = 0
 * @param *ttbl2 Level2 translation table descriptor
 * @return void
 */
void lpaed_stage2_disable_l2_table(union lpaed *ttbl2);

#endif
