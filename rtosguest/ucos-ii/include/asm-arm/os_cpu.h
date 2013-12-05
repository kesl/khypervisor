/*
*********************************************************************************************************
*                                               uC/OS-II
*                                        The Real-Time Kernel
*
*                         (c) Copyright 1992-1999, Jean J. Labrosse, Weston, FL
*                                          All Rights Reserved
*
*
* File         : OS_CPU.H
* By           : Jean J. Labrosse
*********************************************************************************************************
*/

#ifndef _OS_CPU_H
#define _OS_CPU_H

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned int   OS_STK;                   /* Each stack entry is 32-bit wide                    */
typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (PSW = 16 bits) */

#define BYTE           INT8S                     /* Define data types for backward compatibility ...   */
#define UBYTE          INT8U                     /* ... to uC/OS V1.xx.  Not actually needed for ...   */
#define WORD           INT16S                    /* ... uC/OS-II.                                      */
#define UWORD          INT16U
#define LONG           INT32S
#define ULONG          INT32U

/* 
*********************************************************************************************************
*                              Intel 80x86 (Real-Mode, Large Model)
*
* Method #1:  Disable/Enable interrupts using simple instructions.  After critical section, interrupts
*             will be enabled even if they were disabled before entering the critical section.  You MUST
*             change the constant in OS_CPU_A.ASM, function OSIntCtxSw() from 10 to 8.
*
* Method #2:  Disable/Enable interrupts by preserving the state of interrupts.  In other words, if 
*             interrupts were disabled before entering the critical section, they will be disabled when
*             leaving the critical section.  You MUST change the constant in OS_CPU_A.ASM, function 
*             OSIntCtxSw() from 8 to 10.
*
* Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*             would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*             disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to 
*             disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*             into the CPU's status register.
*********************************************************************************************************
*/
#define  OS_CRITICAL_METHOD    3

#if      OS_CRITICAL_METHOD == 1
#define  OS_ENTER_CRITICAL()  
#define  OS_EXIT_CRITICAL()   

/* enable IRQ interrupts */
static inline void enable_int(void)
{
	unsigned long temp;
	__asm__ __volatile__("mrs %0, cpsr\n" "bic %0, %0, #0x80\n" "msr cpsr_c, %0":"=r"(temp)
			     ::"memory");
}


/*
 * disable IRQ/FIQ interrupts
 * returns true if interrupts had been enabled before we disabled them
 */
static inline int disable_int(void)
{
	unsigned long old, temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			     "orr %1, %0, #0xc0\n" "msr cpsr_c, %1":"=r"(old), "=r"(temp)
			     ::"memory");
	return (old & 0x80) == 0;
}


#define  OS_ENTER_CRITICAL()  (disable_int())    /* Disable interrupts                        */
#define  OS_EXIT_CRITICAL()   (enable_int())    /* Enable  interrupts                        */

#endif

#if      OS_CRITICAL_METHOD == 2
#define  OS_ENTER_CRITICAL()		
#define  OS_EXIT_CRITICAL()		
#endif


#define local_irq_disable() asm volatile ( "cpsid i @ local_irq_disable\n" : : : "cc" )
#define local_irq_enable()  asm volatile ( "cpsie i @ local_irq_enable\n" : : : "cc" )

#define local_save_flags(x)                                      \
({                                                               \
    asm volatile ( "mrs %0, cpsr     @ local_save_flags\n"       \
                  : "=r" (x) :: "memory", "cc" );                \
})
#define local_irq_save(x)                                        \
({                                                               \
    local_save_flags(x);                                         \
    local_irq_disable();                                         \
})
#define local_irq_restore(x)                                     \
({                                                               \
    asm volatile (                                               \
            "msr     cpsr_c, %0      @ local_irq_restore\n"      \
            :                                                    \
            : "r" (cpu_sr)                                        \
            : "memory", "cc");                                   \
})


static inline OS_CPU_SR OSCPUSaveSR(void)
{
    OS_CPU_SR _v;

    __asm__ __volatile__(
            "mrs    %0, cpsr \n\t"
            "orr    %0, %0, #0x80 \n\t"
         "msr    cpsr_c, %0 \n\t"
         :"=r" (_v)
         :
         :"memory"
     );

     return _v;
 }


 static inline void OSCPURestoreSR(OS_CPU_SR eflags)
 {
     __asm__ __volatile__(
         "mrs    %0, cpsr \n\t"
         "bic    %0, %0, #0x80 \n\t"
         "msr    cpsr_c, %0 \n\t"
         :
         : "r" (eflags)
         : "memory"
     );
 }


#if      OS_CRITICAL_METHOD == 3
#define  OS_ENTER_CRITICAL()  (cpu_sr = OSCPUSaveSR())    /* Disable interrupts                        */
#define  OS_EXIT_CRITICAL()   (OSCPURestoreSR(cpu_sr))    /* Enable  interrupts                        */
#endif


#define  OS_STK_GROWTH			1

#define  OS_TASK_SW()			OSCtxSw()

/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/
extern void OSTimer0_Period_Setting(void);
extern void OSTimer0_Interrupt_Setting(void);
extern void mmu_init(void);
extern void cpu_mmu_enable(void);
extern void OSCtxSw(void);


#endif
