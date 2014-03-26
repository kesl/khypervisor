#include "tests_malloc.h"
#include "hvmm_trace.h"
#include "lpae.h"
#include "memory.h"
#include "armv7_p15.h"

#include <k-hypervisor-config.h>
#include <log/print.h>

hvmm_status_t hvmm_tests_malloc(void)
{
    HVMM_TRACE_ENTER();
    int *test1;
    int *test2;
    int *test3;
    int *test4;
    int *test5;
    test1 = (int *)memory_alloc(100);
    printh("%s[%d] %x\n", __func__, __LINE__, test1);
    test2 = (int *)memory_alloc(1024);
    printh("%s[%d] %x\n", __func__, __LINE__, test2);
    test3 = (int *)memory_alloc(1025);
    printh("%s[%d] %x\n", __func__, __LINE__, test3);
    test4 = (int *)memory_alloc(750 * 8);
    printh("%s[%d] %x\n", __func__, __LINE__, test4);
    test5 = (int *)memory_alloc(218095616 - 8);
    printh("%s[%d] %x\n", __func__, __LINE__, test5);
    memory_free(test1);
    memory_free(test2);
    memory_free(test3);
    memory_free(test4);
    memory_free(test5);
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}
