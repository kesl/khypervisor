#ifndef __LPAE_H__
#define __LPAE_H__
#include <arch_types.h>

/**
 * @file lpae.h
 *
 * <pre>
 * ARMv7-A LPAE pagetables:3-level trie, mapping 40-bit input to 40-bit output
 * addresses. Tables at all levels have 512 64-bit entries. The bit-shuffling
 * that has the permission bits in branch nodes in a different place from those
 * in leaf nodes seems to be to allow linear pagetable tricks. If we're not
 * doing that then the set of permission bits that's not in use in a given node
 * type can be used as extra software-defined bits.
 * </pre>
 */

/**
 * \defgroup Long_descriptor
 *
 * @brief Long descriptor translation table format
 *
 * Long descriptor translation table format supports the assignment of memory
 * attributes to memory pages, at a granularity of 4KB, across the complete
 * input address range. It also supports the assignment of memory attributes to
 * blocks of memory, where a block can be 2MB or 1GB.
 *
 * In general, a descriptor is one of
 * - a table entry, that points the next-level translation table.
 * - a block entry, that defines the memory properties for the access.
 *
 * Bit[1] indicates the descriptor type, and bit[0] indicates Whether the
 * descriptor is valid.
 *
 * - First-level and second-level descriptor formats
 *   - Block
 * <pre>
 * |bit  |63       52|51    40|39           n|n-1   12|11        2|1|0|
 * |-----|-----------|--------|--------------|--------|-----------|-|-|
 * |value|Upper block|UNK/SBZP|Output address|UNK/SBZP|Lower block|0|1|
 * |     |attributes |        |[39:n]        |        |attributes | | |
 * </pre>
 *   - For the first-level descriptor, n is 30. For the second-level
 *     descriptor, n is 21.
 *   - Table
 * <pre>
 * |bit  |63|62 61|60|59 |58   52|51    40|39            12|11    2|1|0|
 * |-----|--|-----|--|---|-------|--------|----------------|-------|-|-|
 * |value|NS|PAT  |XN|PXN|Ignored|UNK/SBZP|Next-level table|Ignored|1|1|
 * |     |Stage-1 only.  |       |        |address [39:12] |       | | |
 * </pre>
 *
 * - Third-level descriptor formats
 *   - Page
 * <pre>
 * |bits |63       52|51    40|39          12|11       2|1|0|
 * |-----|-----------|--------|--------------|----------|-|-|
 * |value|Upper block|UNK/SBZP|Output address|Lower page|1|1|
 * |     |attributes |        |[39:12]       |attributes| | |
 * </pre>
 *
 * - The attributes in stage-1.
 *   - Lower attributes
 *     - AttrIndx[2:4]
 *       - Stage-1 memory attributes index fileds, for the indicated Memory
 *         Attribute Indirection Register.
 *       - \ref Attribute_Indexes.
 *     - NS[5]
 *       - Non-secure bit. For memory accesses from Secure state, specifies
 *         whether the output address is in Secure or Non-secure memory.
 *     - AP[6:7] -AP[2:1]
 *       - Access Permision bits.
 *         - AR[2] 0 - Read/Write
 *                 1 - Read-only
 *         - AR[1] 0 - Only at PL1
 *                 1 - At any privilege Level (app Level)
 *     - SH[8:9]
 *       - Shareability field.
 *       - \ref Shareability "SH Configuration"
 *     - AF[10]
 *       - The Access Flag.
 *     - nG[11]
 *       - The not global bit. Determines how the translation is marked in the
 *         TLB.
 *
 *   - Upper attributes
 *     - Contigues hint[52]
 *       - A hint bit indicating that 16 adjacent translation table entries
 *         point to contiguous memory regions.
 *     - PXN[53]
 *       - The Privileged eXecute Never bit. Determine whether the region is
 *         executable at PL1.
 *     - XN[54]
 *       - The eXecute Never bit. Determines whether the region is execuatble.
 *
 *   - PXN Table[59]
 *     - Priviliged eXecute Never limit for subsequent level of lookup.
 *   - XN Table[60]
 *     - eXecute Never limit for subsequent level of lookup.
 *   - APT Table[61:62]
 *     - Access Permission limit for subsequent level of lookup.
 *   - NS Table[63]
 *     - For memory access from Secure state, specified the security
 *       level for subsequent level of lookup.
 *     - For memory accesses from Non-secure state, this bit is ignored.
 *
 * - The attributes in stage-2.
 *   - Lower attributes
 *     - MemAttr[2:5]
 *       - Stage-2 memory attributes.
 *       - \ref Memory_attribute "Stage-2 Memory attribute configuration"
 *     - HAP[6:7]
 *       - Stage-2 Access Permissions bit. Read & Write permission.
 * <pre>
 * HAP[2:1] Access permission
 * 00       No access permitted.
 * 01       Read-only. Writes to the region are not permitted.
 * 10       Write-only. Reads from the region are not permitted.
 * 11       Read/Write. The stage-1 permissions determine the access
 *          permissions for the region.
 * </pre>
 *     - SH[8:9]
 *       - Shareability field.
 *       - \ref Shareability "SH Configuration"
 *     - AF[10]
 *       - The Access flag.
 *
 *   - Upper attribute
 *     - Contigues hint[52]
 *       - A hint bit indicating that 16 adjacent translation table entries
 *         point to contiguous memory regions.
 *     - XN[54]
 *       - The eXecute Never bit. Determines whether the region is executable.
 */

/**
 * \defgroup Memory_attribute
 *
 * @brief LPAED stage-2 memory attribute configuration.
 *
 * In the stage-2 translation table descriptors for memory regions and pages,
 * the MemAttr[3:0] and SH[1:0] fields describe the stage-2 memory region
 * attributes.
 *
 * MemAttr[3:2] give a top level definition of the memory type, and of the
 * cacheability of Normal memory region, as behinds.
 * <pre>
 * MemAttr[3:2] |Memory type                 |Cacheability
 * 00           |Strongly-ordered or Device, |Not Applicable
 *              |determined by memAttr[1:0]  |
 * 01           |Normal                      |Outer Non-cacheable
 * 10           |                            |Outer Write-Through Cacheable
 * 11           |                            |Outer Write-Back Cacheable
 * </pre>
 * The encoding of MemAttr[1:0] depends on the Memory type indicated
 * by MemAttr[3:2].
 * <pre>
 * when MemAttr[3:2] == 0b00
 *
 * MemAttr[1:0] Meaning
 * 00           Region is Strongly-ordered memory
 * 01           Region is Device memory
 * 10           UNPREDICTABLE
 * 11           UNPREDICTABLE
 *
 * When MemAttr[3:2] != 0b00
 *
 * MemAttr[1:0] Meaning
 * 00           UNPREDICTABLE
 * 01           Inner Non-cacheable
 * 10           Inner Write-Through Cacheable
 * 11           Inner Write-Back Cacheable
 * </pre>
 */

/**
 * \defgroup LPAE_PAGE_FEATURES
 *
 * This features are used to configure the page address of 4KB size.
 * @{
 */
#define LPAE_PAGE_SHIFT    12
#define LPAE_PAGE_SIZE      (1<<LPAE_PAGE_SHIFT)
#define LPAE_PAGE_MASK      (0xFFF)
/**
 * @}
 */

/**
 * \defgroup LPAE_BLOCK_FEATURES
 *
 * This features are used to configure the bloack addres of 2MB size.
 * @{
 */
#define LPAE_BLOCK_L2_SHIFT 21
#define LPAE_BLOCK_L2_SIZE  (1<<LPAE_BLOCK_L2_SHIFT)
#define LPAE_BLOCK_L2_MASK  (0x1FFFFF)
/**
 * @}
 */

/**
 * \defgroup Attribute_Indexes
 *
 * These are valid in the AttrIndx[2:0] field of an LPAE stage-1 page
 * table entry. They are indexes into the bytes of the MAIRx registers.
 * The 8-bit fields are packed little-endian into MAIR0 and MAIR1.
 * /ref Memory_Attribute_Indirection_Register "MAIRx"
 * <pre>
 *                          ai    encoding
 *   ATTR_IDX_UNCACHED      000   0000 0000  -- Strongly Ordered
 *   ATTR_IDX_BUFFERABLE    001   0100 0100  -- Non-Cacheable
 *   ATTR_IDX_WRITETHROUGH  010   1010 1010  -- Write-through
 *   ATTR_IDX_WRITEBACK     011   1110 1110  -- Write-back
 *   ATTR_IDX_DEV_SHARED    100   0000 0100  -- Device
 *            ??            101
 *            reserved      110
 *   ATTR_IDX_WRITEALLOC    111   1111 1111  -- Write-back write-allocate
 *
 *   ATTR_IDX_DEV_NONSHARED 100   (== ATTR_IDX_DEV_SHARED)
 *   ATTR_IDX_DEV_WC        001   (== ATTR_IDX_BUFFERABLE)
 *   ATTR_IDX_DEV_CACHED    011   (== ATTR_IDX_WRITEBACK)
 *   </pre>
 * @{
 */
#define ATTR_IDX_UNCACHED      0x0
#define ATTR_IDX_BUFFERABLE    0x1
#define ATTR_IDX_WRITETHROUGH  0x2
#define ATTR_IDX_WRITEBACK     0x3
#define ATTR_IDX_DEV_SHARED    0x4
#define ATTR_IDX_WRITEALLOC    0x7
#define ATTR_IDX_DEV_NONSHARED ATTR_IDX_DEV_SHARED
#define ATTR_IDX_DEV_WC        ATTR_IDX_BUFFERABLE
#define ATTR_IDX_DEV_CACHED    ATTR_IDX_WRITEBACK
/** @}*/

/**
 * @brief LPAE page table descriptor.
 *
 * The bits in this format are almost used in Stage-1 translation table.
 * - \ref Long_descriptor "Details"
 *
 * - Descriptoin
 *
 *   - These are used in all kinds of entry.
 *     - valid - Valid mapping
 *     - Table - == 1 - Table Entry, == 0 block or page entry.
 *
 *   - These ten bits are only used in Block entries and are ignored in table
 *     entries. (Stage-1 Lower attributes)
 *     - ai - Attributes Index
 *     - ns - Non-Secure
 *     - User - User-visible (PL0)
 *     - ro - Read-Only
 *     - sh - Shareability
 *     - af - Access Flag
 *     - ng - Not-Global
 *
 *   - The base address must be appropriately aligned for Block entries.
 *     - base - base address of physical address or other Table entry.
 *
 *   - Theses seven bits are only used in Block entries and are ignored.
 *     (Stage-1 Upper block attributes, Used in block & page Descriptor)
 *     - hint - Contiguous hint, In a block of 16 contiguous entries.
 *     - pxn - Privileged-eXecute-Never
 *     - xn - eXecute-Never
 *     - avail - Ignored by Hardware
 *
 *   - These 5 bits are only used in Table entries and are ignored in Block
 *     entries.(Stage-1 only, SBZ at stage-2)
 *     - pxnt - PXNTable
 *     - xnt - XNTable
 *     - apt - APTable
 *     - nst - NSTable
 */
struct lpae_pt {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;     /**< Valid mapping */
    unsigned long table:1;     /**< == 1 in 4k map entries too. */

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
    unsigned long base:28;     /**< Base address of block or next table. */
    unsigned long sbz:12;      /**< Must be zero */

    /* These seven bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long hint:1;      /**< In a block of 16 contiguous entries. */
    unsigned long pxn:1;       /**< Privileged XN */
    unsigned long xn:1;        /**< eXecute Never */
    unsigned long avail:4;     /**< Ignored by hardware */

    /* These 5 bits are only used in Table entries and are ignored in
     * Block entries */
    unsigned long pxnt:1;      /**< Privileged XN */
    unsigned long xnt:1;       /**< eXecute Never */
    unsigned long apt:2;       /**< Access Permissions */
    unsigned long nst:1;       /**<  Not-Secure */
} __attribute__((__packed__));

/**
 * @brief LPAE table descriptor for stage-2 translation
 *
 * This descriptor has almost the same layout to lpae_pt.
 * But it is stage-2 translation descriptor.
 * - \ref Long_descriptor "Details"
 *
 * - Description
 *
 *   - These are used in all kinds of entry.
 *     - valid - Valid mapping
 *     - table - If it is 1, this descriptor is Table descriptor.
 *               Else if it is 0, this descriptor is Block or page descriptor.
 *
 *   - These ten bits are only used in Block entries and are ignored in Table
 *     Entries. (Stage-2 Lower attributes)
 *     - mattr - Memory Attributes
 *     - read - Read Access
 *     - write - Write Access
 *     - sh - Shareability
 *     - af - Access Flag
 *
 *   - The base address must be appropriately aligned for Block entries.
 *     (Stage-2 Upper attributes)
 *     - base - Bass Address of block or next table.
 *   - These six bits are only used in Block entries and are ignored in Table
 *     entries.
 *     - hint - Contiguous Hint, In a block of 16 contiguous entries.
 *     - xn - eXecute Never
 *     - avail - Ignored by Hardware
 */
struct lpae_p2m {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;     /**< Valid mapping */
    unsigned long table:1;     /**< == 1 in 4k map entries too. */

    /* These ten bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long mattr:4;     /**< Memory Attributes */
    unsigned long read:1;      /**< Read access */
    unsigned long write:1;     /**< Write access */
    unsigned long sh:2;        /**< Shareability */
    unsigned long af:1;        /**< Access Flag */
    unsigned long sbz4:1;

    /* The base address must be appropriately aligned for Block entries */
    unsigned long base:28;     /**< Base address of block or next table. */
    unsigned long sbz3:12;

    /* These seven bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long hint:1;      /**< In a block of 16 contiguous entries. */
    unsigned long sbz2:1;
    unsigned long xn:1;        /**< eXecute Never. */
    unsigned long avail:4;     /**< Ignored by hardware. */

    unsigned long sbz1:5;
} __attribute__((__packed__));


/**
 * @brief Walk entry.
 *
 * Walk is the common bits of p2m and pt entries which are needed to
 * simply walk the table. (e.g. for debug).
 *
 * - \ref Long_descriptor "Details"
 *
 * - Description
 *   - valid - Valid mapping
 *   - table - == 1 in 4k map entries too
 *   - pad2 -
 *   - base - Base address of block or next table
 *   - pad1 -
 */
struct lpae_walk {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;     /**< Valid mapping. */
    unsigned long table:1;     /**< == 1 in 4k map entries too. */

    unsigned long pad2:10;

    /* The base address must be appropriately aligned for Block entries */
    unsigned long base:28;     /**< Base address of block or next table. */

    unsigned long pad1:24;
} __attribute__((__packed__));

/**
 * @brief The union of LPAED.
 *
 * For saves the page table descriptor.
 */
union lpaed {
    uint64_t bits;
    struct lpae_pt pt;
    struct lpae_p2m  p2m;
    struct lpae_walk walk;
};

/**
 * @brief Enum values of the lpae stage2 memory attribute.
 *
 * \ref Memory_attribute "Memory attribute configuration"
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
 * for the given physical address.
 *
 * Generates a new stage-2 level 1 block LPAE descriptor which has 1GB block.
 * It returns the descriptor after generating.
 *
 * - Initial configuration
 *   - Valid and block.
 *   - ai bit is configured by parameter 'attr_idx'.
 *   - Non-secure and app-level access are allowed.
 *   - Read/Write are allowed, Outer Shareable.
 *   - Access flag is enabled.
 *   - Not global.
 *   - physical address = pa
 *
 * @param  pa Physical address of the block.
 * @param  attr_idx Attribute index for memory this descriptor.
 * @return  Generated level1 block LPAE descriptor.
 */
union lpaed hvmm_mm_lpaed_l1_block(uint64_t pa, uint8_t attr_idx);
/**
 * @brief Level 2 block, 2MB, entry in LPAE Descriptor format
 * for the given physical address.
 *
 * Generates a new stage-2 level 2 block LPAE descriptor which has 2MB block.
 * It returns the descriptor after generating.
 *
 * - Initial configuration
 *   - Valid and block.
 *   - Memory attribute is configured by parameter 'mattr'.
 *   - read / write are allowed, non-shareable, executable.
 *   - Access flag is enabled.
 *   - physical address = pa
 *
 * @param pa Physical address of the block.
 * @param mattr Memory attribute index.
 * @return Generated level 2 block LPAE descriptor.
 */
union lpaed hvmm_mm_lpaed_l2_block(uint64_t pa,
                enum lpaed_stage2_memattr mattr);
/**
 * @brief Level 1, 1GB, each entry refer level2 page table
 *
 * Generates a new level 1 LPAE table descriptor refers level 2 page table.
 * It returns the descriptor after generating.
 *
 * - Initial configuration
 *   - Valid and table.
 *   - pxn, xn limit for subsequent level of lookup.
 *   - Access Permission limit for subsequent levels of lookup.
 *   - Table address is in the non-secure physical address space.
 *   - physical address = pa
 *
 * @param pa Physical address Refers level2 page table.
 * @return Generated level 1 page table LPAE descriptor.
 */
union lpaed hvmm_mm_lpaed_l1_table(uint64_t pa);
/**
 * @brief Level 2, 2MB, each entry refer level3 page table
 *
 * Generates a new level 2 LPAE table descriptor refers level 3 page table.
 * It returns the descriptor after generating.
 *
 * - Initial configuration
 *   - Valid and table.
 *   - pxn, xn limit for subsequent level of lookup.
 *   - Access Permission limit for subsequent levels of lookup.
 *   - Table address is in the non-secure physical address space.
 *   - physical address = pa
 *
 * @param pa Physical address Refers level3 page table.
 * @return Generated level 2 page table LPAE descriptor.
 */
union lpaed hvmm_mm_lpaed_l2_table(uint64_t pa);
/**
 * @brief Level 3, each entry refer 4KB physical address
 *
 * Generates a new level 3 LPAE page Descriptor refers 4KB physical address.
 * It returns the descriptor after generating.
 *
 * - Initial configuration
 *   - valid bit is configured by parameter 'valid'.
 *   - ai is configured by parameter 'attr_idx'
 *   - Non-secured and user-level access are allowed.
 *   - Read / write are allowed, Outer shareable.
 *   - Access Flag is enabled. executable.
 *   - physical address = pa
 *
 * @param pa 4KB physical address.
 * @param attr_idx Memory attribute index.
 * @param valid Validation of table descriptor.
 * @return Generated level 3 page table LPAE descriptor.
 */
union lpaed hvmm_mm_lpaed_l3_table(uint64_t pa, uint8_t attr_idx,
                uint8_t valid);
/**
 * @brief Configures the stage-1 level 3 table entry.
 *
 * Configure the table validate bit & physical address of level3 table.
 *
 * - Wrtie the page address.
 *   - valid = valid ? 1 : 0
 *   - physical address = pa
 *
 * @param *ttbl3 Pointer of level3 translation table descriptor.
 * @param baddr Target page address.
 * @param valid Validate
 * @return void
 */
void lpaed_stage1_conf_l3_table(union lpaed *ttbl3, uint64_t baddr,
                uint8_t valid);
/**
 * @brief Disables the stage-1 level 3 table entry.
 *
 * Makes invalidate table Descriptor.
 * - valid = 0
 *
 * @param *ttbl3 Pointer of level 3 table descriptor.
 * @return void
 */
void lpaed_stage1_disable_l3_table(union lpaed *ttbl2);
/**
 * @brief Mapping the stage-2 level 2 lpae descriptor to physical address
 * and memory attribute.
 *
 * Reconfigures the descriptor to the level 2 page table descriptor.
 * And mapping the physical address and memory attribute.
 *
 * - State configuration
 *   - valid = 1, table = 1, mattr = mattr & 0x0F, read = 1, write 1
 *   - sh = 0, af = 1, hint = 0, xn = 0 physical address = pa
 *
 * @param *pte Page table descriptor.
 * @param pa Physical address.
 * @param mattr Memory entry.
 * @return void
 */
void lpaed_stage2_map_page(union lpaed *pte, uint64_t pa,
        enum lpaed_stage2_memattr mattr);
/**
 * @brief Configure valid & table bit of the stage-2 level 1 table descriptor.
 * And set the base address.
 *
 * Configure valid bit and table bit.
 * And mapping physical address of the level 2 table.
 *
 * - State configuration
 *   - valid = valid ? 1: 0, table = valid ? 1 : 0
 *   - bass address = baddr
 *
 * @param *ttbl1 The stage-2 level1 translation table descriptor.
 * @param baddr Base address.
 * @param valid Validation
 * @return void
 */
void lpaed_stage2_conf_l1_table(union lpaed *ttbl1, uint64_t baddr,
        uint8_t valid);
/**
 * @brief Configure the stage-2 level 2 table entry's valid & table bit
 * and set the base address.
 *
 * Configure the descriptor's valid bit and table bit.
 * And mapping physical address of the level 3 table.
 *
 * - State configuration
 *   - valid = valid ? 1: 0, table = valid ? 1 : 0
 *   - base address = baddr
 *
 * @param *ttbl2 The stage-2 level2 translation table descriptor.
 * @param baddr Base address.
 * @param valid Validation.
 * @return void
 */
void lpaed_stage2_conf_l2_table(union lpaed *ttbl2, uint64_t baddr,
        uint8_t valid);
/**
 * @brief Enables the stage-2 level2 table entry.
 *
 * Enabling the level 2 descriptor's valid bit and table bit.
 *
 * - State
 *   - valid = 1, table =1
 *
 * @param *ttbl2 The stage-2 Level2 translation table descriptor.
 * @return void
 */
void lpaed_stage2_enable_l2_table(union lpaed *ttbl2);
/**
 * @brief Disables the stage-2 level2 table entry.
 *
 * Disables the valid bit of the descriptor.
 *
 * - State
 *   - valid = 0
 *
 * @param *ttbl2 the stage-2 level2 translation table descriptor.
 * @return void
 */
void lpaed_stage2_disable_l2_table(union lpaed *ttbl2);

#endif
