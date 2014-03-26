#define DEBUG
#include <log/print.h>

#include "traps.h"

#define WFI_WFE_DIRECTION_BIT   0x00000001
#define WFI_WFE_DIRECTION_SHIFT 0 /* Do not use it to shift. */

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
    unsigned int direction;

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
