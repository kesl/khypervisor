#ifndef _TRAP_H_
#define _TRAP_H_

#define MRC(reg, cp, opc1, crn, crm, opc2) \
__asm__ __volatile__ ( \
"   mrc   "   #cp "," #opc1 ", %0,"  #crn "," #crm "," #opc2 "\n" \
: "=r" (reg))

#define MCR(reg, cp, opc1, crn, crm, opc2) \
__asm__ __volatile__ ( \
"   mcr   "   #cp "," #opc1 ", %0,"  #crn "," #crm "," #opc2 "\n" \
: : "r" (reg))

#define MRC15(reg, op1, crn, crm, op2) MRC(reg, p15, op1, crn, crm, op2)
#define MCR15(reg, op1, crn, crm, op2) MCR(reg, p15, op1, crn, crm, op2)

/* TESTS_HCR.TGE */
#define SMC()   __asm__ __volatile__ ("smc #0")

/* TESTS_HCR.TVM */
#define WRITE_SCTLR(reg)    MCR(reg, p15, 0, c1, c0, 0)
#define READ_SCTLR(reg)     MRC(reg, p15, 0, c1, c0, 0)

/* TESTS_HCR.TPU */
#define WRITE_DCCISW(reg)   MCR(reg, p15, 0, c7, c14, 2)
/* #define READ_DCCISW(reg)    MRC(reg, p15, 0, c7, c14, 2) /* WRITE ONLY */

/* TESTS_HCR.TAC */
#define WRITE_ACTLR(reg)    MCR(reg, p15, 0, c1, c0, 1)
#define READ_ACTLR(reg)     MRC(reg, p15, 0, c1, c0, 1)

/* TESTS_HCR.TWE */
#define WFE()   __asm__ __volatile__ ("wfe" : : : "memory")
/* TESTS_HCR.TW1 */
#define WFI()   __asm__ __volatile__ ("wfi" : : : "memory")
/* TESTS_HCR.TSC */
#define SMC()   __asm__ __volatile__ ("smc #0")

#endif
