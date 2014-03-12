#ifndef __MM_H__
#define __MM_H__

#include <hvmm_types.h>
#include "arch_types.h"
#include "lpae.h"

int hvmm_mm_init(void);
/**
 * @brief Hyp mode general-purpose storage allocator
 *
 * Obtain storage from hyp mode heap memory
 * - First, search free block list
 * - Second, if free block list has no list or not enough size of list,
 *   Obtain storage from heap memory
 * - Third, there are list has enough size, return list's pointer
 * @param size Size of space
 * @return Allocated space
 */
void *hmm_malloc(unsigned long size);
/**
 * @brief Free allocated memory
 *
 * Freed the allocated block's header
 * @param addr Target block's address
 * @return void
 */
void hmm_free(void *addr);
/**
 * @brief Unmapping level 3 table entry in virtual address
 *
 * <pre>
 * Get the level 3 table entry from hmm_get_l3_table_entry function
 * And disable this entry's valid & table bit configuration
 * </pre>
 *
 * @param virt Virtual address
 * @param npages Number of pages
 * @return void
 */
void hmm_umap(unsigned long virt, unsigned long npages);
/**
 * @brief Mapping virtual address to level 3 table entries
 *
 * <pre>
 * Mapping the physical address to level 3 table entries
 * This entries are finded by given virtual address
 * And then configure this entries valid bit to 1
 * </pre>
 *
 * @param phys Target physical address
 * @param virt Target virtal address
 * @param npages Number of pages
 * @return void
 */
void hmm_map(unsigned long phys, unsigned long virt, unsigned long npages);

#endif
