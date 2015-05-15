#ifndef __TRAP_H__
#define __TRAP_H__
#include <hvmm_types.h>
#include <guest.h>

/* ESR Exception Class. */
#define TRAP_EC_UNKNOWN        0x00
#define TRAP_EC_WFI_WFE        0x01
#define TRAP_EC_MCR_MRC_CP15   0x03
#define TRAP_EC_MCRR_MRRC_CP15 0x04
#define TRAP_EC_MCR_MRC_CP14   0x05
#define TRAP_EC_LDC_STC_CP14   0x06
#define TRAP_EC_SIMD_FP        0x07
#define TRAP_EC_MCR_MRC_CP10  0x08
#define TRAP_EC_MRRC_CP14  0x0C

#define TRAP_EC_SVC    0x11
#define TRAP_EC_HVC    0x12
#define TRAP_EC_SMC    0x13
#define TRAP_EC_PREFETCH_ABORT_FROM_OTHER_MODE 0x20
#define TRAP_EC_PREFETCH_ABORT_FROM_HYP_MODE   0x21
#define TRAP_EC_DATA_ABORT_FROM_OTHER_MODE 0x24
#define TRAP_EC_DATA_ABORT_FROM_HYP_MODE   0x25

/* ESR Bit Extraction. */
#define ESR_EC_BIT      0xFC000000
#define EXTRACT_EC      26
#define ESR_IL_BIT      0x02000000
#define EXTRACT_IL      25
#define ESR_ISS_BIT     0x01FFFFFF

/*
 * ISS encoding for Data Abort exceptions taken to Hyp mode as beloww
 * ISS[24] : instruction syndrome valid. 0 is invalid information in ISS.
 * 1 is valid information in ISS
 * when ISS[24] is 0, we don't need to extract information
 * from the rest of ISS field
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
 * - ISS[8] is a cache maintenance. For synchronous fault, it should
 * need a cache maintenance.
 * - ISS[7] is a stage 2 fault for a stage 1 translation table walk
 * - ISS[6] is synchronous abort that was caused by a write or read operation
 * - ISS[5:0] is a data fault status code(DFSC)
 * Additional register we should reference a DFSR
 */

#define ISS_VALID                           0x01000000

#define ISS_FSR_MASK                        0x0000003F
#define ISS_TRANS_FAULT_MASK                0x07
#define TRANS_FAULT_LEVEL1                  0x05
#define TRANS_FAULT_LEVEL2                  0x06
#define TRANS_FAULT_LEVEL3                  0x07
#define ACCESS_FAULT_LEVEL0                 0x08
#define ACCESS_FAULT_LEVEL1                 0x09
#define ACCESS_FAULT_LEVEL2                 0x0A
#define ACCESS_FAULT_LEVEL3                 0x0B

#define ISS_WNR_SHIFT                       6
#define ISS_WNR                             (1 << ISS_WNR_SHIFT)

#define ISS_SAS_SHIFT                       22
#define ISS_SAS_MASK                        (0x3 << ISS_SAS_SHIFT)
#define ISS_SAS_BYTE                        0x0
#define ISS_SAS_HWORD                       0x1
#define ISS_SAS_WORD                        0x2
#define ISS_SAS_RESERVED                    0x3

#define ISS_SSE_SHIFT                       21
#define ISS_SSE_MASK                        (0x1 << ISS_SSE_SHIFT)

#define ISS_SRT_SHIFT                       16
#define ISS_SRT_MASK                        (0xf << ISS_SRT_SHIFT)

/* HPFAR */
#define HPFAR_INITVAL                       0x0000000000000000
#define HPFAR_FIPA_MASK                     0xFFFFFFFFF0
#define HPFAR_FIPA_SHIFT                    4
#define HPFAR_FIPA_PAGE_MASK                0x00000FFF
#define HPFAR_FIPA_PAGE_SHIFT               12

/**@brief Handles every exceptions taken from a mode other than Hyp mode.
 * @param regs ARM registers for current virtual machine.
 * @return Returns the result of exceptions.
 */
enum hyp_hvc_result _hyp_sync_service(struct arch_regs *regs);

#endif
