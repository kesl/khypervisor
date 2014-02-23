/*
 * /khypervisor/hypervisor/trap/parse_trapped_instructions.c
 *
 *  Created on: Feb 22, 2014
 *      Author: Wonseok
 */

// Common in EC, HSR[31:30] zero
#define EC_ZERO_CV_BIT 0x01000000
#define EC_ZERO_COND_BIT 0x00E00000
#define EC_ZERO_CV_SHIFT 24
#define EC_ZERO_COND_SHIFT 20
#define IS_VAILD_CV	1

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
// Do not use it to shift.
#define MCR_MRC_DIRECTION_SHIFT 0

void emulate_mcr_mrc_cp15(unsigned int iss, unsigned int il) {
	// If value of EC bit is equal to 0x3, trapped instruction should be handled here.
	unsigned int cv = (iss & EC_ZERO_CV_BIT) >> EC_ZERO_CV_SHIFT;
	unsigned int cond;
	unsigned int Opc2, Opc1, CRn, Rt, CRm, dir;

	if (cv == IS_VAILD_CV) {
		cond = (iss & EC_ZERO_COND_BIT) >> EC_ZERO_COND_SHIFT;
	} else {
		cond = 0x0;
	}
	dir = (iss & MCR_MRC_DIRECTION_BIT);

	Opc2 = (iss & MCR_MRC_OPC2_BIT) >> MCR_MRC_OPC2_SHIFT;
	Opc1 = (iss & MCR_MRC_OPC1_BIT) >> MCR_MRC_OPC1_SHIFT;
	CRn = (iss & MCR_MRC_CRN_BIT) >> MCR_MRC_CRN_SHIFT;
	Rt = (iss & MCR_MRC_RT_BIT) >> MCR_MRC_RT_SHIFT;
	CRm = (iss & MCR_MRC_CRM_BIT) >> MCR_MRC_CRM_SHIFT;


	// MRC CP#, OPC1, REG#, CRn, CRm, OPC2
	if (dir == 0) {
		printf("MCR ");
		printf("p15, %d, Rt%d, c%d, c%d, %d", Opc1, Rt, CRn, CRm, Opc2);
	} else if (dir == 1){
		printf("MRC ");
		printf("p15, %d, Rt%d, c%d, c%d, %d", Opc1, Rt, CRn, CRm, Opc2);
	} else {
		printf("Error: Unknown instructions\n");
	}
}

void emulate_mcr_mrc_cp14(unsigned int iss, unsigned int il) {
	// If value of EC bit is equal to 0x3, trapped instruction should be handled here.
	unsigned int cv = (iss & EC_ZERO_CV_BIT) >> EC_ZERO_CV_SHIFT;
	unsigned int cond;
	unsigned int Opc2, Opc1, CRn, Rt, CRm, dir;

	if (cv == IS_VAILD_CV) {
		cond = (iss & EC_ZERO_COND_BIT) >> EC_ZERO_COND_SHIFT;
	} else {
		cond = 0x0;
	}
	dir = (iss & MCR_MRC_DIRECTION_BIT);

	Opc2 = (iss & MCR_MRC_OPC2_BIT) >> MCR_MRC_OPC2_SHIFT;
	Opc1 = (iss & MCR_MRC_OPC1_BIT) >> MCR_MRC_OPC1_SHIFT;
	CRn = (iss & MCR_MRC_CRN_BIT) >> MCR_MRC_CRN_SHIFT;
	Rt = (iss & MCR_MRC_RT_BIT) >> MCR_MRC_RT_SHIFT;
	CRm = (iss & MCR_MRC_CRM_BIT) >> MCR_MRC_CRM_SHIFT;


	// MRC CP#, OPC1, REG#, CRn, CRm, OPC2
	if (dir == 0) {
		printf("MCR ");
		printf("p14, %d, Rt%d, c%d, c%d, %d", Opc1, Rt, CRn, CRm, Opc2);
	} else if (dir == 1){
		printf("MRC ");
		printf("p14, %d, Rt%d, c%d, c%d, %d", Opc1, Rt, CRn, CRm, Opc2);
	} else {
		printf("Error: Unknown instructions\n");
	}
}

void emulate_mcr_mrc_cp10(unsigned int iss, unsigned int il) {
	// If value of EC bit is equal to 0x3, trapped instruction should be handled here.
	unsigned int cv = (iss & EC_ZERO_CV_BIT) >> EC_ZERO_CV_SHIFT;
	unsigned int cond;
	unsigned int Opc2, Opc1, CRn, Rt, CRm, dir;

	if (cv == IS_VAILD_CV) {
		cond = (iss & EC_ZERO_COND_BIT) >> EC_ZERO_COND_SHIFT;
	} else {
		cond = 0x0;
	}
	dir = (iss & MCR_MRC_DIRECTION_BIT);

	Opc2 = (iss & MCR_MRC_OPC2_BIT) >> MCR_MRC_OPC2_SHIFT;
	Opc1 = (iss & MCR_MRC_OPC1_BIT) >> MCR_MRC_OPC1_SHIFT;
	CRn = (iss & MCR_MRC_CRN_BIT) >> MCR_MRC_CRN_SHIFT;
	Rt = (iss & MCR_MRC_RT_BIT) >> MCR_MRC_RT_SHIFT;
	CRm = (iss & MCR_MRC_CRM_BIT) >> MCR_MRC_CRM_SHIFT;


	// MRC CP#, OPC1, REG#, CRn, CRm, OPC2
	if (dir == 0) {
		printf("MCR ");
		printf("p10, %d, Rt%d, c%d, c%d, %d", Opc1, Rt, CRn, CRm, Opc2);
	} else if (dir == 1){
		printf("VMRS ");
		printf("p10, %d, Rt%d, c%d, c%d, %d", Opc1, Rt, CRn, CRm, Opc2);
	} else {
		printf("Error: Unknown instructions\n");
	}
}

/*
 * When HCR.TWI is set to 1, and the processor is in a Non-secure mode other than Hyp mode,
 * execution of a WFI instruction generates a Hyp Trap exception.
 *
 * Syntax, WFI{<c>}{<q>}
 *
 * if ConditionPassed() then
 * 		EncodingSpecificOperations();
 * 		if HaveVirtExt() && !IsSecure() && !CurrentModeIsHyp() && HCR.TWI == '1' then
 * 			HSRString = Zeros(25);
 * 			HSRString<0> = '0';
 * 			WriteHSR('000001', HSRString);
 * 			TakeHypTrapException();
 * 		else
 * 			WaitForInterrupt();
 *
 * 	In kernel implementation,
 * 		#define wfi()	__asm__ __volatile__ ("wfi" : : : "memory")
 */

/* When HCR.TWE is set to 1, and the processor is in a Non-secure mode other than Hyp mode,
 * execution of a WFE instruction generates a Hyp Trap exception.
 */

// Common Fields in EC, HSR[31:30] zero
#define EC_ZERO_CV_BIT 		0x01000000
#define EC_ZERO_COND_BIT 	0x00E00000
#define EC_ZERO_CV_SHIFT 	24
#define EC_ZERO_COND_SHIFT 	20
#define IS_VAILD_CV			1

#define WFI_WFE_DIRECTION_BIT   0x00000001
#define WFI_WFE_DIRECTION_SHIFT 0 // Do not use it to shift.

void emulate_wfi_wfe(unsigned int iss, unsigned int il) {
	unsigned int cv = (iss & EC_ZERO_CV_BIT) >> EC_ZERO_CV_SHIFT;
	unsigned int cond, direction;

	if (cv == IS_VAILD_CV) {
		cond = (iss & EC_ZERO_COND_BIT) >> EC_ZERO_COND_SHIFT;
	} else {
		cond = 0x0;
	}

	if (il == 0) {
		printf("16-bit Thumb instruction\n");
	} else {
		printf("32-bit ARM instruction\n");
	}

	direction = (iss & WFI_WFE_DIRECTION_BIT);
	if (direction == 0) {
		printf("WFI trapped.\n");
	} else {
		printf("WFE trapped.\n");
	}
}
