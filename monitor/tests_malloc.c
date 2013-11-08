#include "tests_malloc.h"
#include "print.h"
#include "hvmm_trace.h"
#include "lpae.h"
#include "mm.h"
#include "armv7_p15.h"

hvmm_status_t hvmm_tests_malloc(void){
    HVMM_TRACE_ENTER();
    int* test1;
    int* test2;
    int* test3;
    int* test4;
    int* test5;
    test1 = (int*)hmm_malloc(100);
    printh("%s[%d] %x\n", __FUNCTION__, __LINE__, test1);
    test2 = (int*)hmm_malloc(1024);
    printh("%s[%d] %x\n", __FUNCTION__, __LINE__, test2);
    test3 = (int*)hmm_malloc(1025);
    printh("%s[%d] %x\n", __FUNCTION__, __LINE__, test3);
    test4 = (int*)hmm_malloc(750*8);
    printh("%s[%d] %x\n", __FUNCTION__, __LINE__, test4);
    test5 = (int*)hmm_malloc(218095616-8);
    printh("%s[%d] %x\n", __FUNCTION__, __LINE__, test5);
    hmm_free(test1);
    hmm_free(test2);
    hmm_free(test3);
    hmm_free(test4);
    hmm_free(test5);
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}
