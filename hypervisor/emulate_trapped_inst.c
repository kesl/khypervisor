/*
 * /khypervisor/hypervisor/trap/parse_trapped_instructions.c
 *
 *  Created on: Feb 22, 2014
 *      Author: Wonseok
 */

#include "emulate_trapped_inst.h"
#define DEBUG
#include <log/print.h>

void emulate_mcr_mrc_cp15(unsigned int iss, unsigned int il)
{
    /*
     * If value of EC bit is equal to 0x3, trapped
     * instruction should be handled here.
     */
    /* unused variable */
    /* unsigned int cv = (iss & EC_ZERO_CV_BIT) >> EC_ZERO_CV_SHIFT; */

    /* unsigned int cond; */
    unsigned int Opc2, Opc1, CRn, Rt, CRm, dir;

    /* warning: variable ‘cond’ set but not used */
    /*
    if (cv == IS_VAILD_CV)
        cond = (iss & EC_ZERO_COND_BIT) >> EC_ZERO_COND_SHIFT;
    else
        cond = 0x0;
    */
    dir = (iss & MCR_MRC_DIRECTION_BIT);
    Opc2 = (iss & MCR_MRC_OPC2_BIT) >> MCR_MRC_OPC2_SHIFT;
    Opc1 = (iss & MCR_MRC_OPC1_BIT) >> MCR_MRC_OPC1_SHIFT;
    CRn = (iss & MCR_MRC_CRN_BIT) >> MCR_MRC_CRN_SHIFT;
    Rt = (iss & MCR_MRC_RT_BIT) >> MCR_MRC_RT_SHIFT;
    CRm = (iss & MCR_MRC_CRM_BIT) >> MCR_MRC_CRM_SHIFT;
    /* Register ordering MRC CP#, OPC1, REG#, CRn, CRm, OPC2 */
    if (dir == 0) {
        printh("MCR ");
        printh("p15, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else if (dir == 1) {
        printh("MRC ");
        printh("p15, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else
        printh("Error: Unknown instructions\n");
}

void emulate_mcr_mrc_cp14(unsigned int iss, unsigned int il)
{
    /*
     * If value of EC bit is equal to 0x3, trapped
     * instruction should be handled here.
     */
    /* unused variable */
    /* unsigned int cv = (iss & EC_ZERO_CV_BIT) >> EC_ZERO_CV_SHIFT; */
    /* unused variable */
    /* unsigned int cond; */
    unsigned int Opc2, Opc1, CRn, Rt, CRm, dir;
    /* warning: variable ‘cond’ set but not used */
    /* if (cv == IS_VAILD_CV)
        cond = (iss & EC_ZERO_COND_BIT) >> EC_ZERO_COND_SHIFT;
    else
        cond = 0x0;
    */
    dir = (iss & MCR_MRC_DIRECTION_BIT);
    Opc2 = (iss & MCR_MRC_OPC2_BIT) >> MCR_MRC_OPC2_SHIFT;
    Opc1 = (iss & MCR_MRC_OPC1_BIT) >> MCR_MRC_OPC1_SHIFT;
    CRn = (iss & MCR_MRC_CRN_BIT) >> MCR_MRC_CRN_SHIFT;
    Rt = (iss & MCR_MRC_RT_BIT) >> MCR_MRC_RT_SHIFT;
    CRm = (iss & MCR_MRC_CRM_BIT) >> MCR_MRC_CRM_SHIFT;
    /* MRC CP#, OPC1, REG#, CRn, CRm, OPC2 */
    if (dir == 0) {
        printh("MCR ");
        printh("p14, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else if (dir == 1) {
        printh("MRC ");
        printh("p14, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else
        printh("Error: Unknown instructions\n");
}

void emulate_mcr_mrc_cp10(unsigned int iss, unsigned int il)
{
    /*
     * If value of EC bit is equal to 0x3,
     * trapped instruction should be handled here.
     */
    /* unused variable */
    /* unsigned int cv = (iss & EC_ZERO_CV_BIT) >> EC_ZERO_CV_SHIFT; */
    /* unused variable */
    /* unsigned int cond; */
    unsigned int Opc2, Opc1, CRn, Rt, CRm, dir;
    /* warning: variable ‘cond’ set but not used */
    /*
    if (cv == IS_VAILD_CV)
        cond = (iss & EC_ZERO_COND_BIT) >> EC_ZERO_COND_SHIFT;
    else
        cond = 0x0;
    */
    dir = (iss & MCR_MRC_DIRECTION_BIT);
    Opc2 = (iss & MCR_MRC_OPC2_BIT) >> MCR_MRC_OPC2_SHIFT;
    Opc1 = (iss & MCR_MRC_OPC1_BIT) >> MCR_MRC_OPC1_SHIFT;
    CRn = (iss & MCR_MRC_CRN_BIT) >> MCR_MRC_CRN_SHIFT;
    Rt = (iss & MCR_MRC_RT_BIT) >> MCR_MRC_RT_SHIFT;
    CRm = (iss & MCR_MRC_CRM_BIT) >> MCR_MRC_CRM_SHIFT;
    /* MRC CP#, OPC1, REG#, CRn, CRm, OPC2 */
    if (dir == 0) {
        printh("MCR ");
        printh("p10, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else if (dir == 1) {
        printh("VMRS ");
        printh("p10, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else
        printh("Error: Unknown instructions\n");
}

/*
 * When HCR.TWI is set to 1, and the processor is
 * in a Non-secure mode other than Hyp mode,
 * execution of a WFI instruction generates a Hyp Trap exception.
 *
 * Syntax, WFI{<c>}{<q>}
 *
 * if ConditionPassed() then
 *         EncodingSpecificOperations();
 *         if HaveVirtExt() && !IsSecure() && \
 *              !CurrentModeIsHyp() && HCR.TWI == '1' then
 *             HSRString = Zeros(25);
 *             HSRString<0> = '0';
 *             WriteHSR('000001', HSRString);
 *             TakeHypTrapException();
 *         else
 *             WaitForInterrupt();
 *
 *     In kernel implementation,
 *         #define wfi()    __asm__ __volatile__ ("wfi" : : : "memory")
 */

/* When HCR.TWE is set to 1, and the processor is
 * in a Non-secure mode other than Hyp mode,
 * execution of a WFE instruction generates a Hyp Trap exception.
 */

void emulate_wfi_wfe(unsigned int iss, unsigned int il)
{
    /* unused variable */
    /* unsigned int cv = (iss & EC_ZERO_CV_BIT) >> EC_ZERO_CV_SHIFT; */
    /* unused variable */
    /* unsigned int cond; */
    unsigned int direction;
    /* warning: variable ‘cond’ set but not used */
    /*
    if (cv == IS_VAILD_CV)
        cond = (iss & EC_ZERO_COND_BIT) >> EC_ZERO_COND_SHIFT;
    else
        cond = 0x0;
    */
    if (il == 0)
        printh("16-bit Thumb instruction\n");
    else
        printh("32-bit ARM instruction\n");

    direction = (iss & WFI_WFE_DIRECTION_BIT);
    if (direction == 0)
        printh("WFI trapped.\n");
    else
        printh("WFE trapped.\n");

}
