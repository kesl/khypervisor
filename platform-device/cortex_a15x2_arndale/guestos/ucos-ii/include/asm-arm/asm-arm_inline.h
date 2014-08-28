#ifndef __ASM_ARM_INLINE__
#define __ASM_ARM_INLINE__

#define sev()   __asm__ __volatile__ ("sev" : : : "memory")
#define wfe()   __asm__ __volatile__ ("wfe" : : : "memory")
#define wfi()   __asm__ __volatile__ ("wfi" : : : "memory")

#define isb() __asm__ __volatile__ ("isb" : : : "memory")
#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")

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
