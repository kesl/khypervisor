#ifndef __MM_H__
#define __MM_H__

#include <hvmm_types.h>
#include "arch_types.h"
#include "lpae.h"
/**
 * @brief Configures the memory management features.
 *
 * Configure all features of the memory management.
 *
 * - Generate and confgirue hyp mode & virtual mode translation tables.
 * - Configure MAIRx, HMAIRx register.
 *   - \ref Memory_Attribute_Indirection_Register.
 *   - \ref Attribute_Indexes.
 * - Configures Hyp Translation Control Register(HTCR).
 *   - Setting
 *     - Out shareable.
 *     - Normal memory, outer write-through, cacheable.
 *     - Normal memory, inner write-back write-allocate cacheable.
 *     - T0SZ is zero.
 *   - \ref HTCR
 * - Configures Hyper System Control Register(HSCTLR).
 *   - i-cache and alignment checking enable.
 *   - mmu, d-cache, write-implies-xn, low-latency, IRQs disable.
 *   - \ref SCTLR
 * - Configures Hyp Translation Table Base Register(HTTBR).
 *   - Writes the _hmm_pgtable value to base address bits.
 *   - \ref HTTBR
 * - Enable MMU and D-cache in HSCTLR.
 * - Initialize heap area.
 *
 * @return HVMM_STATUS_SUCCESS, Always success.
 */
int hvmm_mm_init(void);
/**
 * @brief Hyp mode general-purpose storage allocator.
 *
 * Obtains the storage from hyp mode heap memory.
 * - First, search free block list.
 * - Second, if free block list has no list or not enough size of list,
 *   Obtains storage from heap memory.
 * - Third, there are list has enough size, return list's pointer.
 *
 * @param size Size of space
 * @return Allocated space
 */
void *hmm_malloc(unsigned long size);
/**
 * @brief Free allocated memory.
 *
 * Freed the header of the allocated block.
 *
 * @param addr The address of target block.
 * @return void
 */
void hmm_free(void *addr);
/**
 * @brief Unmaps level3 table entry in virtual address.
 *
 * Obtains the level3 table entry by using hmm_get_l3_table_entry function.
 * And disables the valid bit and table bit of the entry.
 *
 * @param virt Virtual address.
 * @param npages Number of pages.
 * @return void
 */
void hmm_umap(unsigned long virt, unsigned long npages);
/**
 * @brief Maps virtual address to level 3 table entries.
 *
 * Mapping the physical address to level 3 table entries.
 * This entries are obtained by given virtual address.
 * And then configure this entries valid bit to 1.
 *
 * @param phys Target physical address.
 * @param virt Target virtal address.
 * @param npages Number of pages.
 * @return void
 */
void hmm_map(unsigned long phys, unsigned long virt, unsigned long npages);

#endif
