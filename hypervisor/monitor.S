/*
 * monitor.S - Hyp mode portion of Monitor code
 *
 * Copyright (C) 2013 KESL. All rights reserved.
 *
 */

.syntaxunified
.arch_extension sec
.arch_extension virt
.text


.global monitor_hyp_vectors
/*
 * Monitor Vector Table
 */
.align 5
monitor_hyp_vectors:
.word 0/* reset */
btrap_unhandled/* undef*/
btrap_unhandled /* hypervisor */
btrap_unhandled/* pabt*/
btrap_dabt/* dabt */
btrap_hvc /* hvc */
btrap_irq/* irq */
btrap_unhandled/* fiq*/


trap_hvc:
@ Push registers
push{r0-r12}
mrsr0, spsr_hyp
mrsr1, elr_hyp
push{r0, r1, lr}

@ service other argument values -> _hyp_hvc_service(sp)
movr0, sp
bl_hyp_hvc_service@ r0: HSR

@ r0 = return
tstr0, #1
@ if return == HYP_RET_STAY -> stay in Hyp mode
bne1f

@ Pop registers
pop {r0-r1, lr}
msr    spsr_hyp, r0
msr    elr_hyp, r1
pop     {r0-r12}

@ else if return == HYP_RET_ERET -> Exception Return
eret

1:
@ Pop registers
pop {r0-r1, lr}
tst r0, #0x1f
msrnespsr_hyp, r0
msrelr_hyp, r1
pop     {r0-r12}
@ stay in Hyp mode
mrslr, elr_hyp
movpc, lr

trap_dabt:
@ Push registers
push{r0-r12}
mrsr0, spsr_hyp
mrsr1, elr_hyp
push{r0, r1, lr}

@ service other argument values -> _hyp_trap_dabort(sp)
movr0, sp
bl_hyp_trap_dabort@ r0: HSR

@ if (return == 0) execute a conditional code
@ tstr0, #1
@ <conditional code>

@ Pop registers
pop {r0-r1, lr}
msrspsr_hyp, r0
msrelr_hyp, r1
pop     {r0-r12}
eret

trap_irq:
@ Push registers
push{r0-r12}
mrsr0, spsr_hyp
mrsr1, elr_hyp
push{r0, r1, lr}

@ service other argument values -> _hyp_trap_irq(sp)
movr0, sp
bl_hyp_trap_irq@ r0: HSR

@ Pop registers
pop {r0-r1, lr}
msr    spsr_hyp, r0
msr    elr_hyp, r1
pop     {r0-r12}
eret

trap_unhandled:
@ Push registers
push{r0-r12}
mrs     r0, spsr_hyp
mrs     r1, elr_hyp
push{r0, r1, lr}

@ service other argument values -> _hyp_trap_unhandled(sp)
movr0, sp
bl_hyp_trap_unhandled@ r0: HSR

@ Pop registers
pop {r0-r1, lr}
msr     spsr_hyp, r0
msr     elr_hyp, r1
pop     {r0-r12}
eret

.global __mon_switch_to_guest_context
__mon_switch_to_guest_context:@ struct arch_regs *r0
ldmfdr0!, {r1, r3, lr}
    msr     spsr_hyp, r1
    msr     elr_hyp, r3
    ldm     r0, {r0-r12}
clrex
eret
.type __mon_switch_to_guest_context, %function



