#ifndef __TRAP_H__
#define __TRAP_H__

#include <hvmm_types.h>

/* HSR Exception Class. */
#define TRAP_EC_ZERO_UNKNOWN        0x00
#define TRAP_EC_ZERO_WFI_WFE        0x01
#define TRAP_EC_ZERO_MCR_MRC_CP15   0x03
#define TRAP_EC_ZERO_MCRR_MRRC_CP15 0x04
#define TRAP_EC_ZERO_MCR_MRC_CP14   0x05
#define TRAP_EC_ZERO_LDC_STC_CP14   0x06
#define TRAP_EC_ZERO_HCRTR_CP0_CP13 0x07
#define TRAP_EC_ZERO_MRC_VMRS_CP10  0x08
#define TRAP_EC_ZERO_BXJ    0x0A
#define TRAP_EC_ZERO_MRRC_CP14  0x0C
#define TRAP_EC_NON_ZERO_SVC    0x11
#define TRAP_EC_NON_ZERO_HVC    0x12
#define TRAP_EC_NON_ZERO_SMC    0x13
#define TRAP_EC_NON_ZERO_PREFETCH_ABORT_FROM_OTHER_MODE 0x20
#define TRAP_EC_NON_ZERO_PREFETCH_ABORT_FROM_HYP_MODE   0x21
#define TRAP_EC_NON_ZERO_DATA_ABORT_FROM_OTHER_MODE 0x24
#define TRAP_EC_NON_ZERO_DATA_ABORT_FROM_HYP_MODE   0x25

/* HSR Bit Extraction. */
#define HSR_EC_ZERO     0xC0000000
#define EXTRACT_EC_ZERO 30
#define HSR_EC_BIT      0xFC000000
#define EXTRACT_EC      26
#define HSR_IL_BIT      0x02000000
#define EXTRACT_IL      25
#define HSR_ISS_BIT     0x01FFFFFF

/**
 * @brief Returns saved ARM registers
 * <br> Arm registers are temporary saved registers in file
 * @return The saved ARM registers
 */
struct arch_regs *trap_saved_regs(void);

/**
 * @brief Handles data abort exception
 * <br> however this handler used to trap into hvc instead of conducting data abort
 * @param iss iss is instruction specific syndrome and bits of HSR register 0~24
 * @param regs ARM registers
 * <br> which includes 13 general purpose register r0-r12, 1 Stack Pointer (SP),
 * <br> 1 Link Register (LR), 1 Program Counter (PC)
 * <br> this fuction uses current ARM registers to save and dump as parameter
 * @return If vitual device is successfully emulated,
 * <br> it will be return HVMM_STATUS_SUCCESS, otherwise failed
 */
hvmm_status_t trap_hvc_dabort(unsigned int iss, struct arch_regs *regs);

/**
 * @brief Handles HVC exception called by virtual guest os or other
 * <br> when virtual guest excuted a hvc instruction, this function will be called
 * @param regs ARM registers
 * @return Result of HVC exception
 * <br> if return value is HYP_RESULT_STAY, it will be stay in hypervisor mode
 * <br> otherwise exception routine will be exit and return
 */
enum hyp_hvc_result _hyp_hvc_service(struct arch_regs *regs);

#endif
