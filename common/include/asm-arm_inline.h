#ifndef __ASM_ARM_INLINE__
#define __ASM_ARM_INLINE__

#define sev()   __asm__ __volatile__ ("sev" : : : "memory")
#define wfe()   __asm__ __volatile__ ("wfe" : : : "memory")
#define wfi()   __asm__ __volatile__ ("wfi" : : : "memory")

#define isb() __asm__ __volatile__ ("isb" : : : "memory")
#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")

/*
 * CP15 Barrier instructions
 * Please note that we have separate barrier instructions in ARMv7
 * However, we use the CP15 based instructtions because we use
 * -march=armv5 in U-Boot
 */
#define CP15ISB asm volatile ("mcr     p15, 0, %0, c7, c5, 4" : : "r" (0))
#define CP15DSB asm volatile ("mcr     p15, 0, %0, c7, c10, 4" : : "r" (0))
#define CP15DMB asm volatile ("mcr     p15, 0, %0, c7, c10, 5" : : "r" (0))

#define irq_enable() asm volatile("cpsie i" : : : "memory")
#define asm_clz(x)      ({ uint32_t rval; asm volatile(\
                                " clz %0, %1\n\t" \
                                : "=r" (rval) : "r" (x) : ); rval; })


#define irq_disable() asm volatile ("cpsid i" : : : "memory")

#define irq_disabled() ({ unsigned long tf; \
                asm volatile (" mrs     %0, cpsr\n\t" \
                          : "=r" (tf) \
                          :  \
                          : "memory", "cc"); \
                (tf & CPSR_IRQ_DISABLED) ? TRUE : FALSE; \
                })


#define irq_save(flags)    do { \
                    asm volatile ( \
                    "mrs     %0, cpsr\n\t" \
                    "cpsid   i\n\t" \
                    : "=r" ((flags)) : : "memory", "cc"); \
                    } while (0)


#define irq_restore(flags) do { \
                    asm volatile (" msr     cpsr_c, %0" \
                    : : "r" ((flags)) : "memory", "cc"); \
                    } while (0)


#endif
