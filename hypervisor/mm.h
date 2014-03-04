#ifndef __MM_H__
#define __MM_H__

#include <hvmm_types.h>
#include "arch_types.h"
#include "lpae.h"

/**
 * @brief Initialize All the Memory Management Feature
 *
 * Initialize Memory Management Features<br>
 * - First, Initialize Virtual Memory TTB<br>
 * - Second, Initialize Host Monitor Memory TTB<BR>
 * - Third, Initialize Control Register<br>
 *   - MAIR, HMAIR, HTCR, HRC<br>
 * - Fourth, Set host Monitor Memory TTB(_hmm_pgtable) to HTTBR<br>
 * - Fifth, Enable PL2 Stage 1 MMU by configuring HSCTLR
 * - Sixth, Initialize heap Area<br>
 * @param void
 * @return HVMM_STATUS_SUCCESS
 */
int hvmm_mm_init(void);
/**
 * @brief Malloc, get size of space
 *
 * Get dynamic Memory from Heap Area
 * @param size size of space
 * @return allocated space
 */
void *hmm_malloc(unsigned long size);
/**
 * @brief Free Allocated Memory
 *
 * Free memory
 * @param * addr Target Area's address
 * @return void
 */
void hmm_free(void *addr);
/**
 * @brief Unmapping level 3 table entry in virtual address
 *
 * @param virt virtual address
 * @param npages number of pages
 * @return void
 */
void hmm_umap(unsigned long virt, unsigned long npages);
/**
 * @brief Mapping virtual address to level 3 table entry
 *
 * maping physical address to level 3 table entry<br>
 * that be finded by virtual address and flush TLB
 * @param phys target physical address
 * @param virt target virtal address
 * @param npages number of pages
 * @return void
 */
void hmm_map(unsigned long phys, unsigned long virt, unsigned long npages);

#endif
