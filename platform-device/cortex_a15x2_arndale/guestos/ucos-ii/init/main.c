#include <stdio.h>

#include "includes.h"
#include "asm-arm/timer.h"
#include "asm-arm/irq.h"

#define  TASK_STK_SIZE 512
#define  N_TASKS 8

OS_STK TaskStk[N_TASKS][TASK_STK_SIZE];

OS_STK TaskStartStk[TASK_STK_SIZE];
char TaskData[N_TASKS];

void TaskStart(void *data);
void Task(void *data);


void Task(void *data)
{
	char task = *(char *)data;
    OS_TCB MYTaskData;
    
    
    for (;;) {
    OSTaskQuery((int)task + 11, &MYTaskData);
          printf("Task%d() RUNNING\n", (int)task);
        OSTimeDlyHMSM(0, 0, 1, 0);
    }
}

OS_TCB MYTaskData[8];

void TaskStart(void *data)
{
    int i;

    init_time();
    asm volatile ( "cpsie if" );
    OSStatInit(); /* Initialize uC/OS-II's statistics */


    for (i = 0; i < N_TASKS; i++) {
        TaskData[i] = i;
        OSTaskCreate(Task, (void *) &TaskData[i],
                &TaskStk[i][TASK_STK_SIZE - 1], 11 + i);
    }

    printf("OSStatInit\n");

    //int i, j;
    while (1) {
        OSCtxSwCtr = 0;
        OSTimeDlyHMSM(0, 0, 4, 0);

        for(i = 0; i <N_TASKS; i++){
            OSTaskQuery(11+i, &(MYTaskData[i]));
        }

        printf("---------------------------------\n");
        printf("   # Task \tstatus \tPrio \n"); 
        printf("---------------------------------\n");
        for(i = 0; i < N_TASKS; i++){
            printf("   Task%d() : \t%s \t%d\n", i, "READY", MYTaskData[i].OSTCBPrio);
        }
        printf("---------------------------------\n");
        printf("#Tasks :%5d\tCPU Usage :%3d%\n", OSTaskCtr, OSCPUUsage);
        printf("#Task switch/sec :%5d\n", OSCtxSwCtr);
        printf("\n");
    }

}


int main(void)
{
    OSInit();
    init_IRQ();
    printf("TaskStart\n");

    printf("\nInit Done\n");

    /*Initialize uC/OS-II*/
    OSTaskCreate(TaskStart, (void *) 0, &TaskStartStk[TASK_STK_SIZE - 1], 7);

    printf("\nStart multitasking \n");
    OSStart(); /* Start multitasking */

    return 0;
}

