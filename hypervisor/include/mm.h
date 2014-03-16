#ifndef __MM_H__
#define __MM_H__

#include <hvmm_types.h>
#include "arch_types.h"
#include "lpae.h"

int hvmm_mm_init(void);
void *hmm_malloc(unsigned long size);
void hmm_free(void *addr);
void hmm_umap(unsigned long virt, unsigned long npages);
void hmm_map(unsigned long phys, unsigned long virt, unsigned long npages);

#endif
