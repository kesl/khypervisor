#ifndef __TRAP_H__
#define __TRAP_H__

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
 * @brief get saved registers in trap
 * @param void
 * @return struct arch_regs * saved registers
 */
struct arch_regs *trap_saved_regs(void);
#endif
