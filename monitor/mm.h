#ifndef __MM_H__
#define __MM_H__

#include <hvmm_types.h>
#include "arch_types.h"
#include "lpae.h"

#define HEAP_ADDR 0xF0200000
#define HEAP_SIZE 0xEE00000
#define HEAP_TABLE_ENTRIES 119*512

int hvmm_mm_init(void);

lpaed_t* mm_get_l3_table_heap(void);      

#endif
