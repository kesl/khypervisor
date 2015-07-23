#ifndef __SMP_H__
#define __SMP_H__

#include "arch_types.h"
#include "armv8_processor.h"
#include <asm-arm_inline.h>

#define MPIDR_MASK 0xFFFFFFFFFF
#define MPIDR_AFFI0_MASK 0xFF
#define MPIDR_AFFI1_MASK 0xFF00
#define MPIDR_AFFI2_MASK 0xFF0000
#define MPIDR_AFFI3_MASK 0xFF00000000

/**
 * @brief Gets current CPU ID of the Symmetric MultiProcessing(SMP).
 *
 * Read the value from Multiprocessor ID Register(MPIDR) and obtains the CPU ID
 * by masking.
 * - Coretex-A15
 *   - MPIDR[ 7: 0] - Affinity 0
 *   - MPIDR[15: 8] - Affinity 1
 *   - MPIDR[23:16] - Affinity 2
 * @return The current CPU ID.
 */
static inline uint64_t smp_processor_id(void)
{
    return read_sr64(MPIDR_EL1) & MPIDR_MASK & MPIDR_AFFI0_MASK;
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
"1: ldxr   %0, [%1]\n"
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
    unsigned int tmp;

    asm volatile(
    "2:   ldaxr   %w0, %1\n"
    "     cbnz    %w0, 1f\n"
    "     stxr    %w0, %w2, %1\n"
    "     cbnz    %w0, 2b\n"
    "1:\n"
    : "=&r" (tmp), "+Q" (lock->lock)
    : "r" (1)
    : "cc", "memory");

    return !tmp;
}

static inline void spin_unlock(spinlock_t *lock)
{
    asm volatile(
"   str %1, %0\n"
    : "=Q" (lock->lock)
    : "r" (0)
    : "memory");
}

#define smp_spin_lock(lock, flags)         \
    do {                                    \
        irq_save((flags));         \
        spin_trylock(lock);           \
    } while (0)

#define smp_spin_unlock(lock, flags)       \
    do {                                    \
        spin_unlock(lock);         \
        irq_restore((flags));         \
    } while (0)

#endif
