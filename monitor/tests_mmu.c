#include "tests_mmu.h"
#include "print.h"
#include "hvmm_trace.h"
#include "lpae.h"
#include "mm.h"
#include "armv7_p15.h"

hvmm_status_t hvmm_tests_mmu(void){
    HVMM_TRACE_ENTER();
    volatile uint32_t *addr;
    hmm_malloc(0);
    addr = (uint32_t *)0xf2000000;
    *addr = 10;
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}
