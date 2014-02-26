#ifndef __LPAE_H__
#define __LPAE_H__
#include <arch_types.h>

#define LPAE_PAGE_SHIFT    12
#define LPAE_PAGE_SIZE      (1<<LPAE_PAGE_SHIFT)
#define LPAE_PAGE_MASK      (0xFFF)

#define LPAE_BLOCK_L2_SHIFT 21
#define LPAE_BLOCK_L2_SIZE  (1<<LPAE_BLOCK_L2_SHIFT)
#define LPAE_BLOCK_L2_MASK  (0x1FFFFF)

/*
 * Attribute Indexes.
 *
 * These are valid in the AttrIndx[2:0] field of an LPAE stage 1 page
 * table entry. They are indexes into the bytes of the MAIR*
 * registers, as defined above.
 *
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

struct lpae_pt {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;     /* Valid mapping */
    unsigned long table:1;     /* == 1 in 4k map entries too */

    /* These ten bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long ai:3;        /* Attribute Index */
    unsigned long ns:1;        /* Not-Secure */
    unsigned long user:1;      /* User-visible */
    unsigned long ro:1;        /* Read-Only */
    unsigned long sh:2;        /* Shareability */
    unsigned long af:1;        /* Access Flag */
    unsigned long ng:1;        /* Not-Global */

    /* The base address must be appropriately aligned for Block entries */
    unsigned long base:28;     /* Base address of block or next table */
    unsigned long sbz:12;      /* Must be zero */

    /* These seven bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long hint:1;      /* In a block of 16 contiguous entries */
    unsigned long pxn:1;       /* Privileged-XN */
    unsigned long xn:1;        /* eXecute-Never */
    unsigned long avail:4;     /* Ignored by hardware */

    /* These 5 bits are only used in Table entries and are ignored in
     * Block entries */
    unsigned long pxnt:1;      /* Privileged-XN */
    unsigned long xnt:1;       /* eXecute-Never */
    unsigned long apt:2;       /* Access Permissions */
    unsigned long nst:1;       /* Not-Secure */
} __attribute__((__packed__));

/* The p2m tables have almost the same layout, but some of the permission
 * and cache-control bits are laid out differently (or missing) */
struct lpae_p2m {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;     /* Valid mapping */
    unsigned long table:1;     /* == 1 in 4k map entries too */

    /* These ten bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long mattr:4;     /* Memory Attributes */
    unsigned long read:1;      /* Read access */
    unsigned long write:1;     /* Write access */
    unsigned long sh:2;        /* Shareability */
    unsigned long af:1;        /* Access Flag */
    unsigned long sbz4:1;

    /* The base address must be appropriately aligned for Block entries */
    unsigned long base:28;     /* Base address of block or next table */
    unsigned long sbz3:12;

    /* These seven bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long hint:1;      /* In a block of 16 contiguous entries */
    unsigned long sbz2:1;
    unsigned long xn:1;        /* eXecute-Never */
    unsigned long avail:4;     /* Ignored by hardware */

    unsigned long sbz1:5;
} __attribute__((__packed__));


/*
 * Walk is the common bits of p2m and pt entries which are needed to
 * simply walk the table (e.g. for debug).
 */
struct lpae_walk {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;     /* Valid mapping */
    unsigned long table:1;     /* == 1 in 4k map entries too */

    unsigned long pad2:10;

    /* The base address must be appropriately aligned for Block entries */
    unsigned long base:28;     /* Base address of block or next table */

    unsigned long pad1:24;
} __attribute__((__packed__));

union lpaed {
    uint64_t bits;
    struct lpae_pt pt;
    struct lpae_p2m  p2m;
    struct lpae_walk walk;
};

enum lpaed_stage2_memattr {
    LPAED_STAGE2_MEMATTR_SO = 0x0,   /* Strongly Ordered */
    LPAED_STAGE2_MEMATTR_DM = 0x1,   /* Device memory */
    LPAED_STAGE2_MEMATTR_NORMAL_ONC = 0x4,  /* Outer Non-cacheable */
    LPAED_STAGE2_MEMATTR_NORMAL_OWT = 0x8,
    LPAED_STAGE2_MEMATTR_NORMAL_OWB = 0xC,
    LPAED_STAGE2_MEMATTR_NORMAL_INC = 0x1,
    LPAED_STAGE2_MEMATTR_NORMAL_IWT = 0x2,
    LPAED_STAGE2_MEMATTR_NORMAL_IWB = 0x3,
};

union lpaed hvmm_mm_lpaed_l1_block(uint64_t pa, uint8_t attr_idx);
union lpaed hvmm_mm_lpaed_l2_block(uint64_t pa,
                enum lpaed_stage2_memattr mattr);
union lpaed hvmm_mm_lpaed_l1_table(uint64_t pa);
union lpaed hvmm_mm_lpaed_l2_table(uint64_t pa);
union lpaed hvmm_mm_lpaed_l3_table(uint64_t pa, uint8_t attr_idx,
                uint8_t valid);
void lpaed_stage1_conf_l3_table(union lpaed *ttbl3, uint64_t baddr,
                uint8_t valid);
void lpaed_stage1_disable_l3_table(union lpaed *ttbl2);
void lpaed_stage2_map_page(union lpaed *pte, uint64_t pa,
        enum lpaed_stage2_memattr mattr);
void lpaed_stage2_conf_l1_table(union lpaed *ttbl1, uint64_t baddr,
        uint8_t valid);
void lpaed_stage2_conf_l2_table(union lpaed *ttbl2, uint64_t baddr,
        uint8_t valid);
void lpaed_stage2_enable_l2_table(union lpaed *ttbl2);
void lpaed_stage2_disable_l2_table(union lpaed *ttbl2);

#endif
