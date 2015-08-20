#ifndef __SMP_H__
#define __SMP_H__

#include "arch_types.h"
#include "armv7_p15.h"
#include <asm-arm_inline.h>

#define MPIDR_MASK 0xFFFFFF
#define MPIDR_CPUID_MASK 0xFF

/**
 * @brief Gets current CPU ID of the Symmetric MultiProcessing(SMP).
 *
 * Read the value from Multiprocessor ID Register(MPIDR) and obtains the CPU ID
 * by masking.
 * - Coretex-A15
 *   - MPIDR[1:0] - CPUID - 0, 1, 2, OR 3
 *   - MPIDR[7:2] - Reserved, Read as zero
 * @return The current CPU ID.
 */
static inline uint32_t smp_processor_id(void)
{
    return read_mpidr() & MPIDR_MASK & MPIDR_CPUID_MASK;
}

#define __ARCH_SPIN_LOCK_UNLOCKED 0

#define ALT_SMP(smp, up)                    \
    "9998:  " smp "\n"                  \
    "   .pushsection \".alt.smp.init\", \"a\"\n"    \
    "   .long   9998b\n"                \
    "   " up "\n"                   \
    "   .popsection\n"

#define SEV     ALT_SMP("sev", "nop")
#define WFE(cond)   ALT_SMP("wfe" cond, "nop")

#define mb()    dmb()
#define rmb()   dmb()
#define wmb()   dmb()

#define smp_mb()    dmb()
#define smp_rmb()   dmb()
#define smp_wmb()   dmb()

static inline void dsb_sev(void)
{
    __asm__ __volatile__ (
        "dsb\n"
        SEV
    );
}

#define spin_is_locked(x)      ((x)->lock != 0)

typedef struct {
    volatile unsigned int lock;
} spinlock_t;

#define SPIN_LOCK_UNLOCKED(l)   \
    { .lock = __ARCH_SPIN_LOCK_UNLOCKED, }

#define DEFINE_SPINLOCK(l) spinlock_t l = SPIN_LOCK_UNLOCKED(l);

static inline void spin_lock(spinlock_t *lock)
{
    unsigned long tmp;

    __asm__ __volatile__(
"1: ldrex   %0, [%1]\n"
"   teq %0, #0\n"
    WFE("ne")
"   strexeq %0, %2, [%1]\n"
"   teqeq   %0, #0\n"
"   bne 1b"
    : "=&r" (tmp)
    : "r" (&lock->lock), "r" (1)
    : "cc");

    smp_mb();
}

static inline int spin_trylock(spinlock_t *lock)
{
    unsigned long tmp;

    __asm__ __volatile__(
"   ldrex   %0, [%1]\n"
"   teq %0, #0\n"
"   strexeq %0, %2, [%1]"
    : "=&r" (tmp)
    : "r" (&lock->lock), "r" (1)
    : "cc");

    if (tmp == 0) {
        smp_mb();
        return 1;
    } else {
        return 0;
    }
}

static inline void spin_unlock(spinlock_t *lock)
{
    smp_mb();

    __asm__ __volatile__(
"   str %1, [%0]\n"
    :
    : "r" (&lock->lock), "r" (0)
    : "cc");

    dsb_sev();
}

#define smp_spin_lock(lock, flags)         \
    do {                                    \
        irq_save((flags));         \
        spin_lock(lock);           \
    } while (0)

#define smp_spin_unlock(lock, flags)       \
    do {                                    \
        spin_unlock(lock);         \
        irq_restore((flags));         \
    } while (0)

#endif
