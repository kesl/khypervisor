#define DEBUG
#include <log/print.h>

#include "traps.h"

/* Common in EC, HSR[31:30] zero */
#define EC_ZERO_CV_BIT      0x01000000
#define EC_ZERO_COND_BIT    0x00F00000
#define EC_ZERO_CV_SHIFT    24
#define EC_ZERO_COND_SHIFT  20
#define IS_VAILD_CV         1

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

void emulate_access_to_cp15(unsigned int iss, unsigned int il)
{
    /* If value of EC bit is equal to 0x3, trapped instruction should be handled here. */
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

    /* Print instruction with register ordering */
    if (dir == 0) {
        printh("MCR ");
        printh("p15, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else if (dir == 1) {
        printh("MRC ");
        printh("p15, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else {
        printh("Error: Unknown instructions\n");
    }
}

void emulate_access_to_cp14(unsigned int iss, unsigned int il)
{
    /* If value of EC bit is equal to 0x3, trapped instruction should be handled here. */
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

    /* Print instruction with register ordering */
    if (dir == 0) {
        printh("MCR ");
        printh("p14, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else if (dir == 1) {
        printh("MRC ");
        printh("p14, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else {
        printh("Error: Unknown instructions\n");
    }
}

void emulate_access_to_cp10(unsigned int iss, unsigned int il)
{
    /* If value of EC bit is equal to 0x3, trapped instruction should be handled here. */
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

    /* Print instruction with register ordering */
    if (dir == 0) {
        printh("MCR ");
        printh("p10, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else if (dir == 1) {
        printh("VMRS ");
        printh("p10, %d, Rt%d, c%d, c%d, %d\n", Opc1, Rt, CRn, CRm, Opc2);
    } else {
        printh("Error: Unknown instructions\n");
    }
}
