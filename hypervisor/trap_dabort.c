#include <arch_types.h>
#include <armv7_p15.h>
#include <vdev.h>
#include <hvmm_trace.h>
#include "context.h"
#include "trap_dabort.h"

#define DEBUG
#include <log/print.h>

/*
 * ISS encoding for Data Abort exceptions taken to Hyp mode as beloww
 * ISS[24] : instruction syndrome valid. 0 is invalid information in ISS. 1 is valid information in ISS
 * when ISS[24] is 0, we don't need to extract information from the rest of ISS field
 * when ISS[24] is 1, we need to extract information from ISS[26:13]
 *
 * ISS[26:13] is consist of 10 parts. Details as below
 *
 * - ISS[23:22] is an access size.
 * - e.g. byte, hardword, word
 * - ISS[21] is a sign extend
 * - e.g. 1 is not sign, 1 is sign
 * - ISS[20] is reserved
 * - ISS[19:16] is for register transfer ?
 * - ISS[15:10] is reserved
 * - ISS[9] is an external abort type. It is IMPLEMENTATION_DEFINED
 * - ISS[8] is a cache maintenance. For synchronous fault, it should need a cache maintenance.
 * - ISS[7] is a stage 2 fault for a stage 1 translation table walk
 * - ISS[6] is synchronous abort that was caused by a write or read operation
 * - ISS[5:0] is a data fault status code(DFSC)
 * Additional register we should reference a DFSR
 */

#define ISS_VALID                        0x01000000

#define ISS_FSR_MASK                      0x0000003F
#define ISS_TRANS_FAULT_MASK            0x07
#define TRANS_FAULT_LEVEL1                0x05
#define TRANS_FAULT_LEVEL2                0x06
#define TRANS_FAULT_LEVEL3                0x07
#define ACCESS_FAULT_LEVEL0                0x08
#define ACCESS_FAULT_LEVEL1                0x09
#define ACCESS_FAULT_LEVEL2                0x0A
#define ACCESS_FAULT_LEVEL3                0x0B

#define ISS_WNR_SHIFT                   6
#define ISS_WNR                         (1 << ISS_WNR_SHIFT)

#define ISS_SAS_SHIFT                   22
#define ISS_SAS_MASK                    (0x3 << ISS_SAS_SHIFT)
#define ISS_SAS_BYTE                    0x0
#define ISS_SAS_HWORD                   0x1
#define ISS_SAS_WORD                    0x2
#define ISS_SAS_RESERVED                0x3

#define ISS_SSE_SHIFT                    21
#define ISS_SSE_MASK                    (0x1 << ISS_SSE_SHIFT)

#define ISS_SRT_SHIFT                   16
#define ISS_SRT_MASK                    (0xf << ISS_SRT_SHIFT)

/* HPFAR */
#define HPFAR_INITVAL                   0x00000000
#define HPFAR_FIPA_MASK                 0xFFFFFFF0
#define HPFAR_FIPA_SHIFT                4
#define HPFAR_FIPA_PAGE_MASK                0x00000FFF
#define HPFAR_FIPA_PAGE_SHIFT               12

/*
   Handles data abort case trapped into hvc, not dabort
 */
hvmm_status_t trap_hvc_dabort(unsigned int iss, struct arch_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    /* far, fipa, il */
    uint32_t far = read_hdfar();
    uint32_t fipa;
    uint32_t sas, srt, wnr;
    HVMM_TRACE_ENTER();
    printh("trap_hvc_dabort: hdfar:%x hpfar:%x\n", far, read_hpfar());
    fipa = (read_hpfar() & HPFAR_FIPA_MASK) >> HPFAR_FIPA_SHIFT;
    fipa = fipa << HPFAR_FIPA_PAGE_SHIFT;
    fipa = fipa | (far & HPFAR_FIPA_PAGE_MASK);
    sas = (iss & ISS_SAS_MASK) >> ISS_SAS_SHIFT;
    srt = (iss & ISS_SRT_MASK) >> ISS_SRT_SHIFT;
    wnr = (iss & ISS_WNR) ? 1 : 0;
    if ((iss & ISS_VALID) && ((iss & ISS_FSR_MASK) < 8)) {
        /*
           vdev emulates read/write, update pc, update destination register
         */
        result = vdev_emulate(fipa, wnr, (vdev_access_size_t) sas, srt, regs);
        if (result != HVMM_STATUS_SUCCESS) {
            printh("trap_dabort: emulation failed guest pc:%x\n", regs->pc);
            /* Let the guest continue by increasing pc */
            regs->pc += 4;
        }
    } else {
        printh("trap_dboart: fipa=0x%x\n", fipa);
        result = HVMM_STATUS_BAD_ACCESS;
    }
    if (result != HVMM_STATUS_SUCCESS) {
        printh("- INSTR: %s[%d] r%d [%x]\n", wnr ? "str" : "ldr", (sas + 1) * 8, srt, fipa);
    }
    switch (iss & ISS_FSR_MASK) {
    case TRANS_FAULT_LEVEL1:
    case TRANS_FAULT_LEVEL2:
    case TRANS_FAULT_LEVEL3:
        break;
    case ACCESS_FAULT_LEVEL1:
    case ACCESS_FAULT_LEVEL2:
    case ACCESS_FAULT_LEVEL3: {
        printh("ACCESS fault %d\n", iss & ISS_FSR_MASK);
    }
    break;
    default:
        break;
    }
    HVMM_TRACE_EXIT();
    return result;
}
