/*
 * /khypervisor/hypervisor/trap/parse_trapped_instructions.h
 *
 *  Created on: Feb 22, 2014
 *      Author: Wonseok
 */

/* Common in EC, HSR[31:30] zero */
#define EC_ZERO_CV_BIT 0x01000000
#define EC_ZERO_COND_BIT 0x00E00000
#define EC_ZERO_CV_SHIFT 24
#define EC_ZERO_COND_SHIFT 20
#define IS_VAILD_CV    1

#define MCR_MRC_OPC2_BIT        0x000E0000
#define MCR_MRC_OPC1_BIT        0x0001C000
#define MCR_MRC_CRN_BIT         0x00003C00
#define MCR_MRC_RT_BIT          0x000001E0
#define MCR_MRC_CRM_BIT         0x0000001E
#define MCR_MRC_DIRECTION_BIT   0x00000001

#define MCR_MRC_OPC2_SHIFT      17
#define MCR_MRC_OPC1_SHIFT      14
#define MCR_MRC_CRN_SHIFT       10
#define MCR_MRC_RT_SHIFT        5
#define MCR_MRC_CRM_SHIFT       1
/* Do not use it to shift. */
#define MCR_MRC_DIRECTION_SHIFT 0

void emulate_mcr_mrc_cp15(unsigned int iss, unsigned int il);
void emulate_mcr_mrc_cp14(unsigned int iss, unsigned int il);
void emulate_mcr_mrc_cp10(unsigned int iss, unsigned int il);

#define WFI_WFE_DIRECTION_BIT   0x00000001
#define WFI_WFE_DIRECTION_SHIFT 0 /* Do not use it to shift. */

void emulate_wfi_wfe(unsigned int iss, unsigned int il);
