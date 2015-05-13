#ifndef __ARMV8_PROCESSOR_H__
#define __ARMV8_PROCESSOR_H__
#include "arch_types.h"

#define read_sr64(name)       ({ uint64_t rval; asm volatile (\
                              " mrs %0, "#name \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_sr64(val, name) asm volatile(\
                              "msr "#name", %0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_sr32(name)       ({ uint32_t rval; asm volatile (\
                              " mrs %0, "#name \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_sr32(val, name) asm volatile(\
                              "msr "#name", %0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define invalidate_tlb(name)   asm volatile(\
                              "tlbi "#name)
#define HCR_RW      (0x1 << 31)
#define HCR_TGE     (0x1 << 27)
#define HCR_AMO     (0x1 << 5)
#define HCR_FMO     0x8
#define HCR_IMO     0x10
#define HCR_VI      (0x1 << 7)

#define local_irq_enable()  asm volatile(\
                            "msr daifclr, 0x2" \
                               : : : )
#define local_irq_disable() asm volatile(\
                            "msr daifset, 0x2" \
                               : : : )

#define local_fiq_enable()  asm volatile(\
                            "msr daifclr, 0x1" \
                               : : : )
#define local_fiq_disable() asm volatile(\
                            "msr daifset, 0x1" \
                               : : : )

#define local_serror_enable()  asm volatile(\
                            "msr daifclr, 0x4" \
                               : : : )
#define local_serror_disable() asm volatile(\
                            "msr daifset, 0x4" \
                               : : : )
/* */
/* Generic ARM Registers  */
#define read_vbar()             read_sr64(vbar_el1)
#define write_vbar(val)         write_sr64(val, vbar_el1)
#define read_hvbar()            read_sr64(vbar_el2)
#define write_hvbar(val)        write_sr64(val, vbar_el2)

#define read_ttbr0()            read_sr64(ttbr0_el1)
#define write_ttbr0(val)        write_sr64(val, ttbr0_el1)
#define read_ttbr1()            read_sr64(ttbr1_el1)
#define write_ttbr1(val)        write_sr64(val, ttbr1_el1)
#define read_ttbcr()            read_sr64(tcr_el1)
#define write_ttbcr(val)        write_sr64(val, tcr_el1)

#define read_mair0()
#define read_mair1()
#define write_mair0(val)
#define write_mair1(val)

#define read_hmair0()
#define read_hmair1()
#define write_hmair0()
#define write_hmair1()

#define read_mair()             read_sr64(mair_el1)
#define write_mair(val)         write_sr64(val, mair_el1)
#define read_hmair()            read_sr64(mair_el2)
#define write_hmair(val)        write_sr64(val, mair_el2)

#define read_hsr()              read_sr32(esr_el2)

#define read_htcr()             read_sr32(tcr_el2)
#define write_htcr(val)         write_sr32(val, tcr_el2)

#define read_hsctlr()           read_sr32(sctlr_el2)
#define write_hsctlr(val)       write_sr32(val, sctlr_el2)
#define read_sctlr()            read_sr32(sctlr_el1)
#define write_sctlr(val)        write_sr32(val, sctlr_el1)

#define read_httbr()            read_sr64(ttbr0_el2)
#define write_httbr(val)        write_sr64(val, ttbr0_el2)

#define read_vtcr()             read_sr32(vtcr_el2)
#define write_vtcr(val)         write_sr32(val, vtcr_el2)

#define read_vttbr()            read_sr64(vttbr_el2)
#define write_vttbr(val)        write_sr64(val, vttbr_el2)

#define read_hcr()              read_sr64(hcr_el2)
#define write_hcr(val)          write_sr64(val, hcr_el2)

#define read_midr()             read_sr32(midr_el1)
#define read_mpidr()            read_sr64(mpidr_el1)

/* Generic Timer */

#define read_cntfrq()           read_sr32(cntfrq_el0)
#define write_cntfrq(val)       write_sr32(val, cntfrq_el0)

#define read_cnthctl()          read_sr32(cnthctl_el2)
#define write_cnthctl(val)      write_sr32(val, cnthctl_el2)

#define read_cnthp_ctl()        read_sr32(cnthp_ctl_el2)
#define write_cnthp_ctl(val)    write_sr32(val, cnthp_ctl_el2)

#define read_cnthp_cval()       read_sr64(cnthp_cval_el2)
#define write_cnthp_cval(val)   write_sr64(val, cnthp_cval_el2)

#define read_cnthp_tval()       read_sr32(cnthp_tval_el2)
#define write_cnthp_tval(val)   write_sr32(val, cnthp_tval_el2)

#define read_cntkctl()          read_sr32(cntkctl_el1)
#define write_cntkctl(val)      write_sr32(val, cntkctl_el1)

#define read_cntp_ctl()         read_sr32(cntp_ctl_el0)
#define write_cntp_ctl(val)     write_sr32(val, cntp_ctl_el0)

#define read_cntp_cval()        read_sr64(cntp_cval_el0)
#define write_cntp_cval(val)    write_sr64(val, cntp_cval_el0)

#define read_cntp_tval()        read_sr32(cntp_tval_el0)
#define write_cntp_tval(val)    write_sr32(val, cntp_tval_el0)

#define read_cntpct()           read_sr64(cntpct_el0)

#define read_cntv_ctl()         read_sr32(cntv_ctl_el0)
#define write_cntv_ctl(val)     write_sr32(val, cntv_ctl_el0)

#define read_cntv_cval()        read_sr64(cntv_cval_el0)
#define write_cntv_cval(val)    write_sr64(val, cntv_cval_el0)

#define read_cntv_tval()        read_sr32(cntv_tval_el0)
#define write_cntv_tval(val)    write_sr32(val, cntv_tval_el0)

#define read_cntvct()           read_sr64(cntvct_el0)

#define read_cntvoff()          read_sr64(cntvoff_el2)
#define write_cntvoff(val)      write_sr64(val, cntvoff_el2)

#define read_hdfar()            read_sr64(far_el2)
#define read_hifar
#define write_hdfar(val)        write_sr64(val, far_el2)
#define write_hifar(val)

#define read_hpfar()            read_sr64(hpfar_el2)
#define write_hpfar(val)        write_sr64(val, hpfar_el2)

#endif
