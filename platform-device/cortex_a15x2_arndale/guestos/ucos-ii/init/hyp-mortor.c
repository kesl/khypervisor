#include <stdio.h>

#include "includes.h"
#include "asm-arm/timer.h"
#include "asm-arm/irq.h"

#define  TASK_STK_SIZE 512
OS_EVENT *UART0Sem;

OS_STK MotorFlagControlTaskStk[TASK_STK_SIZE];
OS_STK MotorRailControlTaskStk[TASK_STK_SIZE];
OS_STK MotorLockControlTaskStk[TASK_STK_SIZE];
OS_STK TaskStartStk[TASK_STK_SIZE];

void TaskStart(void *data);
void MotorFlagControlTask(void *data);
void MotorRailControlTask(void *data);
void MotorLockControlTask(void *data);


#define MSG_MemSet   0x01
#define MSG_DegSet   0x10
#define MSG_DegMem   0x11
#define MSG_DegMemX   0x12
#define MSG_DegMemT   0x14
#define MSG_SpdSet   0x20
#define MSG_SpdMem   0x21
#define MSG_SpdMemX   0x22
#define MSG_DegSpdSet  0x30
#define MSG_DegSpdMem  0x31
#define MSG_TickSet   0x40
#define MSG_Report   0x50
#define MSG_ReportTimeSet 0x51
#define MSG_ReportTimeSetX 0x52
#define MSG_AccelDivSet  0x60
#define MSG_AccelDivSetX 0x61
#define MSG_PwmStart  0x82
#define MSG_PwmStop   0x83

void uart_tx_char(char c);
int serial_getc_dev(void);

void EnableTimer()
{
#define VDEV_TIMER_BASE    0x3FFFE000

	    volatile unsigned int *base = (unsigned int *) VDEV_TIMER_BASE;
		    *base = 0;
}

inline void nrm_delay(void)
{
	volatile int i = 0;
	for( i = 0; i < 0x0000fFF; i++);
}


INT8U UART0_getc(void)
{
	return (INT8U)serial_getc_dev();
}

void UART0_putc(INT8U data)
{
	uart_tx_char(data);
}

void Ack(void)
{
	while (UART0_getc() != 0xAC)
		;
}

void WaitReport(void)
{
	while (UART0_getc() != 0xED)
		;
}

void DegSet(INT32U did, INT32U sid, INT32U acc, INT32U degree)
{
	UART0_putc((char) 0x5);
	UART0_putc((char) 0);
	UART0_putc((char) MSG_DegSet);
	UART0_putc((char) sid);
	UART0_putc((char) acc);
	UART0_putc((char) degree);
	Ack();
}

void MemSet(INT32U did)
{
	UART0_putc((char) 0x2);
	UART0_putc((char) 0);
	UART0_putc((char) MSG_MemSet);
	Ack();
}

void Report(INT32U did)
{
	UART0_putc((char) 0x2);
	UART0_putc((char) 0);
	UART0_putc((char) MSG_Report);
	Ack();
}

void MotorAbsTurn(INT32U id, INT32U angle)
{
	INT8U err;
	OSSemPend(UART0Sem, 0, &err);
	DegSet(0, id, 1, angle);
	MemSet(0);
	Report(0);
	WaitReport();
	OSSemPost(UART0Sem);
}

void MotorRelativeTurn(INT32U id, INT32U angle)
{
	INT8U err;
	OSSemPend(UART0Sem, 0, &err);
	DegSet(0, id, 0, angle);
	MemSet(0);
	Report(0);
	WaitReport();
	OSSemPost(UART0Sem);
}

void MotorFlagControlTask(void *data)
{
	INT32U sid, stage;
	sid = 0;
	stage = 0;

	for (;;) {
		if (!stage) {
			MotorAbsTurn(sid, 120);
			printf("motor control 140 sid : %d\n", sid);
		} else {
			MotorAbsTurn(sid, 0);
			printf("motor control 0 sid : %d\n", sid);
		}
		sid = (sid + 1) % 6;
		if (!sid) {
			stage = (stage + 1) & 0x1;
		}
		nrm_delay();
		OSTimeDly(100);
	}
}

void MotorRailControlTask(void *data)
{
	INT32U sid, stage;
	sid = 6;
	stage = 0;

	for (;;) {
		if (!stage) {
			MotorAbsTurn(sid, 180);
			printf("motor control 140 sid : %d\n", sid);
			stage = 1;
		} else {
			MotorAbsTurn(sid, 0);
			printf("motor control 0 sid : %d\n", sid);
			stage = 0;
		}
        OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

void MotorLockControlTask(void *data)
{
	INT32U sid, stage;
	sid = 7;
	stage = 0;

	for (;;) {
		if (!stage) {
			MotorAbsTurn(sid, 180);
			printf("motor control 140 sid : %d\n", sid);
			stage = 1;
		} else {
			MotorAbsTurn(sid, 0);
			printf("motor control 0 sid : %d\n", sid);
			stage = 0;
		}
        OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

void TaskStart(void *data)
{
    int i;

    init_time();
    EnableTimer();
    OSStatInit(); /* Initialize uC/OS-II's statistics */

	UART0Sem = OSSemCreate(1);

    OSTaskCreate(MotorFlagControlTask, (void *) 0, &MotorFlagControlTaskStk[TASK_STK_SIZE - 1], 10);
    OSTaskCreate(MotorRailControlTask, (void *) 0, &MotorRailControlTaskStk[TASK_STK_SIZE - 1], 8);
    OSTaskCreate(MotorLockControlTask, (void *) 0, &MotorLockControlTaskStk[TASK_STK_SIZE - 1], 9);

    printf("OSStatInit\n");

    while (1) {
        OSCtxSwCtr = 0;
        OSTimeDlyHMSM(0, 0, 30, 0);
    }

}


int main(void)
{
    OSInit();
    init_IRQ();
    printf("TaskStart\n");

    printf("\n Init Done\n");

    /*Initialize uC/OS-II*/
    OSTaskCreate(TaskStart, (void *) 0, &TaskStartStk[TASK_STK_SIZE - 1], 7);

    printf("\n Start multitasking \n");
    OSStart(); /* Start multitasking */

    return 0;
}

