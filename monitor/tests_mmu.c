#include "tests_mmu.h"
#include "print.h"
#include "hvmm_trace.h"
#include "lpae.h"
#include "mm.h"
#include "armv7_p15.h"

lpaed_t* heap_table_p;

hvmm_status_t hvmm_tests_l2_table(void){
    HVMM_TRACE_ENTER();
    int test_index = 0;
    uint32_t htcr = read_htcr();
    int t0sz = htcr & 0x00000003;
    int n = 5-t0sz;
    volatile uint32_t *addr = (uint32_t *)(HEAP_ADDR + ((test_index) * 0x1000));
    uint32_t vaddr = 0xf0200000;
    uint32_t ttbr = read_httbr();
    volatile char *vuart = (char *)0xff000000;

    heap_table_p = mm_get_l3_table_heap();
    // heap memory region -> 0xF0200000 ~ 0xFF000000
    // heap_table_p index region -> 0 ~ 119*512


    //test
    *vuart = 't';
    *vuart = 't';
    *vuart = 't';
    *vuart = 't';
    *vuart = 't';
    *vuart = 't';
    *vuart = 't';
    *vuart = 't';
    *vuart = 't';
    *vuart = '\n';
    *vuart = '\r';
            
    
    *addr = 10;
    printh("%d\n", *addr);

    asm("dsb");

    heap_table_p[test_index].pt.valid = 0; 

    asm("dsb");
    asm("isb");

    *addr = 100;
    printh("%d\n", *addr);

    
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}
