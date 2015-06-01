#ifndef __ASM_ARM_INLINE__
#define __ASM_ARM_INLINE__

#define sev()   __asm__ __volatile__ ("sev" : : : "memory")
#define wfe()   __asm__ __volatile__ ("wfe" : : : "memory")
#define wfi()   __asm__ __volatile__ ("wfi" : : : "memory")

#define isb() __asm__ __volatile__ ("isb" : : : "memory")
#define dsb(scop) __asm__ __volatile__ ("dsb " #scop: : : "memory")
#define dmb(scop) __asm__ __volatile__ ("dmb " #scop: : : "memory")

/*
 * CP15 Barrier instructions
 * Please note that we have separate barrier instructions in ARMv7
 * However, we use the CP15 based instructtions because we use
 * -march=armv5 in U-Boot
 */

#define irq_enable() asm volatile("msr daifclr, 0x2" : : : "memory")
#define asm_clz(x)      ({ uint32_t rval; asm volatile(\
                                " clz %0, %1\n\t" \
                                : "=r" (rval) : "r" (x) : ); rval; })


#define irq_disable() asm volatile ("msr daifset, 0x2" : : : "memory")

#define irq_disabled() ({ unsigned int tf; \
                asm volatile (" mrs     %0, daif\n\t" \
                          : "=r" (tf) \
                          :  \
                          : "memory", "cc"); \
                (tf & CPSR_IRQ_DISABLED) ? TRUE : FALSE; \
                })


#define irq_save(flags)    do { \
                    asm volatile ( \
                    "mrs     %0, daif\n\t" \
                    "msr daifset 0x2\n\t" \
                    : "=r" ((flags)) : : "memory", "cc"); \
                    } while (0)


#define irq_restore(flags) do { \
                    asm volatile (" msr     daif, %0" \
                    : : "r" ((flags)) : "memory", "cc"); \
                    } while (0)


#endif
