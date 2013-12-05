#include <asm-arm/board.h>

#define C15_C0_M		0x0001	
#define C15_C0_A		0x0002	
#define C15_C0_C		0x0004	
#define C15_C0_W		0x0008	
#define C15_C0_B		0x0080	
#define C15_C0_S		0x0100	
#define C15_C0_R		0x0200	
#define C15_C0_Z		0x0800	
#define C15_C0_I		0x1000	
#define C15_C0_V		0x2000	

#define ARM_MODE_MASK	0x1F
#define ARM_MODE_SVC	0x13
#define ARM_MODE_UND	0x1B
#define ARM_MODE_ABT	0x17
#define ARM_MODE_IRQ	0x12
#define ARM_MODE_FIQ	0x11
#define ARM_MODE_USR	0x10
#define ARM_MODE_SYS	0x1F
#define ARM_NOIRQ_SVC	0xD3
#define ARM_NOIRQ_UND	0xDB
#define ARM_NOIRQ_ABT	0xD7
#define ARM_NOIRQ_IRQ	0xD2
#define ARM_NOIRQ_FIQ	0xD1
#define ARM_NOIRQ_USR	0xD0
#define ARM_NOIRQ_SYS	0xDF

#define ARM_IRQ_BIT	0x080
#define ARM_FIQ_BIT	0x040
#define ARM_A_BIT	0x100 

.globl start_up
.extern main

start_up:
	bl main

