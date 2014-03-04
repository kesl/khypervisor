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
 * @return int HVMM_STATUS_SUCCESS
 */
int hvmm_mm_init(void);
/**
 * @brief malloc, get size of space
 *
 * Get dynamic Memory from Heap Area
 * @param unsigned long size size of space
 * @return void * allocated space
 */
void *hmm_malloc(unsigned long size);
/**
 * @brief Free Allocated Memory
 *
 * Free memory
 * @param void* addr Target Area's address
 * @return void
 */
void hmm_free(void *addr);
/**
 * @brief unmapping level 3 table entry in virtual address
 *
 * @param unsigned long virt virtual address
 * @param unsigned long npages number of pages
 * @return void
 */
void hmm_umap(unsigned long virt, unsigned long npages);
/**
 * @brief mapping virtual address to level 3 table entry
 *
 * maping physical address to level 3 table entry<br>
 * that be finded by virtual address and flush TLB
 * @param unsigned long phys target physical address
 * @param unsigned long virt target virtal address
 * @param unsigned long npages number of pages
 * @return void
 */
void hmm_map(unsigned long phys, unsigned long virt, unsigned long npages);

#endif
