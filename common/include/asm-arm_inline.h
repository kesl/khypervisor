#ifndef __ASM_ARM_INLINE__
#define __ASM_ARM_INLINE__

#define isb() asm volatile("isb" : : : "memory")
#define dsb() asm volatile("dsb" : : : "memory")
#define irq_enable() asm volatile("cpsie i" : : : "memory")
#define asm_clz(x)      ({ uint32_t rval; asm volatile(\
                                " clz %0, %1\n\t" \
                                : "=r" (rval) : "r" (x) :); rval;})
#endif
