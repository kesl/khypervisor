.extern OSTCBCur
.extern OSTaskSwHook
.extern OSTCBHighRdy
.extern OSPrioCur
.extern OSPrioHighRdy

#define NOINT       0xc0

.globl OSIntCtxSw

.globl	OSCtxSw
OSCtxSw:
	stmfd	sp!,{lr}			// push resume address
	stmfd	sp!,{r0 - r12, lr}		// push rest context
	mrs	r0,SPSR
	bic	r0, r0, #0x80			// F bit Clear

	stmfd	sp!,{r0}			// push CPSR

	ldr	r0,=OSTCBCur			// r0 <= &OSTCBCur
	ldr	r0,[r0]				// r0 <=  OSTCBCur
	str	sp,[r0]				// OSTCBCur->OSTCBStkPtr = sp

	bl	OSTaskSwHook			// Call user defined task switch hook

	ldr	r0,=OSTCBCur			// r0 <= &OSTCBCur
	ldr	r1,=OSTCBHighRdy		// r1 <= &OSTCBHighRdy

	ldr	r2,[r1]				// r2 <=  OSTCBHighRdy
	str	r2,[r0]				// OSTCBCur = OSTCBHighRdy

	ldr	r0,=OSPrioCur			// r0 <= &OSPrioCur
	ldr	r1,=OSPrioHighRdy		// r1 <= &OSPrioHighRdy

	ldrb	r3,[r1]				// r3 <=  OSPrioHighRdy
	strb	r3,[r0]				// OSPrioCur = OSPrioHighRdy

	ldr	sp,[r2]				// sp <=  OSTCBHighRdy->OSTCBStkPtr
	ldmfd	sp!,{r0}			// restore SP...

	msr	SPSR,r0
	ldmfd   sp!,{r0 - r12, lr,pc}		// Load task's context & SPSR-> CPSR & Run task


OSIntCtxSw:
	add	sp,sp,#16
	ldr 	r0,=OSTCBCur
	ldr	r0,[r0]
	str	sp,[r0]
	ldr	r0,=OSTCBCur
	ldr	r1,=OSTCBHighRdy
	ldr	r2,[r1]
	str	r2,[r0]
	ldr	r0,=OSPrioCur
	ldr	r1,=OSPrioHighRdy
	ldrb	r3,[r1]
	strb	r3,[r0]
	ldr	sp,[r2]
	ldmfd	sp!,{r0}
	msr	spsr,r0
	ldmfd	sp!,{r0 - r12,lr,pc}^ 

.extern	OSTaskSwHook
.extern	OSRunning
.extern	OSTCBHighRdy
.extern	OSStartHighRdy
.globl OSStartHighRdy

OSStartHighRdy:
	ldr	r0,=OSRunning			/* Indicate that multitasking has started */
	mov	r1,#1
	strb	r1,[r0]
	ldr	r0,=OSTCBHighRdy		/* r0 <= &OSTCBHighRdy */
	ldr	r0,[r0]					/* r0 <=  OSTCBHighRdy */
	ldr	sp,[r0]					/* sp <=  OSTCBHighRdy->OSTCBStkPtr */
	ldmfd	sp!,{r0}			/* restore SP... */
	msr 	CPSR,r0
	ldmfd   sp!,{r0 - r12, lr , pc}		/* Load task's context & Run task */

.global  OSCPUSaveSR
OSCPUSaveSR:
        mrs r0,CPSR
        orr r1,r0,#0x80
        msr CPSR_c,r1
        bx lr


.global  OSCPURestoreSR
OSCPURestoreSR:
        msr CPSR_c,r0
        bx lr
