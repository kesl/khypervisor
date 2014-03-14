#define DEBUG
#include <log/print.h>
#include <guest.h>
#include <trap.h>
#include <vdev.h>
#include <armv7_p15.h>

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

#define WFI_WFE_DIRECTION_BIT   0x00000001
#define WFI_WFE_DIRECTION_SHIFT 0 /* Do not use it to shift. */

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

static int32_t vdev_cp_read(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    unsigned int ec = info->ec;
    unsigned int hsr = read_hsr();

    switch (ec) {
    case TRAP_EC_ZERO_UNKNOWN:
        printh("Unknown reason: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_WFI_WFE:
        printh("Trapped WFI or WFE instruction: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MCR_MRC_CP15:
        printh("Trapped MCR or MRC access to CP15: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MCRR_MRRC_CP15:
        printh("Trapped MCRR or MRRC access to CP15: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MCR_MRC_CP14:
        printh("Trapped MCR or MRC access to CP14: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_LDC_STC_CP14:
        printh("Trapped LDC or STC access to CP14: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_HCRTR_CP0_CP13:
        printh("HCPTR-trapped access to CP0-CP13: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MRC_VMRS_CP10:
        printh(
            "Trapped MRC or VMRS access to CP10, for ID group traps: 0x%08x\n",
            hsr);
        break;
    case TRAP_EC_ZERO_BXJ:
        printh("Trapped BXJ instruction: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MRRC_CP14:
        printh("Trapped MRRC access to CP14: 0x%08x\n", hsr);
        break;
    case TRAP_EC_NON_ZERO_SVC:
        printh("Supervisor Call exception routed to Hyp mode: 0x%08x\n", hsr);
        break;
    }

    return 0;
}

static int32_t vdev_cp_write(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    unsigned int ec = info->ec;
    unsigned int hsr = read_hsr();

    switch (ec) {
    case TRAP_EC_ZERO_UNKNOWN:
        printh("Unknown reason: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_WFI_WFE:
        printh("Trapped WFI or WFE instruction: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MCR_MRC_CP15:
        printh("Trapped MCR or MRC access to CP15: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MCRR_MRRC_CP15:
        printh("Trapped MCRR or MRRC access to CP15: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MCR_MRC_CP14:
        printh("Trapped MCR or MRC access to CP14: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_LDC_STC_CP14:
        printh("Trapped LDC or STC access to CP14: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_HCRTR_CP0_CP13:
        printh("HCPTR-trapped access to CP0-CP13: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MRC_VMRS_CP10:
        printh(
            "Trapped MRC or VMRS access to CP10, for ID group traps: 0x%08x\n",
            hsr);
        break;
    case TRAP_EC_ZERO_BXJ:
        printh("Trapped BXJ instruction: 0x%08x\n", hsr);
        break;
    case TRAP_EC_ZERO_MRRC_CP14:
        printh("Trapped MRRC access to CP14: 0x%08x\n", hsr);
        break;
    case TRAP_EC_NON_ZERO_SVC:
        printh("Supervisor Call exception routed to Hyp mode: 0x%08x\n", hsr);
        break;
    }

    return 0;
}

static int32_t vdev_cp_check(struct arch_vdev_trigger_info *info,
                        struct arch_regs *regs)
{
    uint32_t ec = info->ec;

    if (ec == TRAP_EC_ZERO_MCR_MRC_CP15 ||
        ec == TRAP_EC_ZERO_MCRR_MRRC_CP15 ||
        ec == TRAP_EC_ZERO_MCR_MRC_CP14 ||
        ec == TRAP_EC_ZERO_HCRTR_CP0_CP13 ||
        ec == TRAP_EC_ZERO_MRC_VMRS_CP10 ||
        ec == TRAP_EC_ZERO_MRRC_CP14 ||
        ec == TRAP_EC_ZERO_MCR_MRC_CP14 ||
        ec == TRAP_EC_ZERO_LDC_STC_CP14)
        return 0;

    return VDEV_NOT_FOUND;
}

static hvmm_status_t vdev_cp_reset_values(void)
{
    printh("vdev init:'%s'\n", __func__);
    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_cp_ops = {
    .init = vdev_cp_reset_values,
    .check = vdev_cp_check,
    .read = vdev_cp_read,
    .write = vdev_cp_write,
};

struct vdev_module _vdev_cp_module = {
    .name = "K-Hypervisor vDevice CP Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_cp_ops,
};

hvmm_status_t vdev_cp_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(VDEV_LEVEL_HIGH, &_vdev_cp_module);
    if (result == HVMM_STATUS_SUCCESS)
        printh("vdev registered:'%s'\n", _vdev_cp_module.name);
    else {
        printh("%s: Unable to register vdev:'%s' code=%x\n",
                __func__, _vdev_cp_module.name, result);
    }

    return result;
}
vdev_module_high_init(vdev_cp_init);
