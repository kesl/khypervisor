#include <stdio.h>

#include "includes.h"
#include "asm-arm/timer.h"
#include "asm-arm/irq.h"

#define  TASK_STK_SIZE 512
#define  N_TASKS 4

OS_STK TaskStk[N_TASKS][TASK_STK_SIZE];

OS_STK TaskStartStk[TASK_STK_SIZE];
char TaskData[N_TASKS];

void TaskStart(void *data);
void Task(void *data);

OS_EVENT *Tick1MsSem;
OS_EVENT *Tick2MsSem;
OS_EVENT *Tick4MsSem;
OS_EVENT *Tick8MsSem;

OS_TCB MYTaskData[8];

void MsTickHanlder()
{

}

void EnableTimer()
{
#define VDEV_TIMER_BASE    0x3FFFE000

    volatile unsigned int *base = (unsigned int *) VDEV_TIMER_BASE;
    *base = 0;
}

void Task1Ms(void *data)
{
	char task = *(char *)data;
    OS_TCB MYTaskData;
    INT8U err;
    struct exynos5_gpio_part2 *gpio2;

    gpio2 = (struct exynos5_gpio_part2 *) EXYNOS5_GPIO_PART2_BASE;

    for (;;) {
        s5p_gpio_set_value(&gpio2->h1, 1, 1);
        OSTimeDly(1);
        OSSemPost(Tick2MsSem);
        s5p_gpio_set_value(&gpio2->h1, 1, 0);
        OSSemPend(Tick1MsSem, 0, &err);
    }
}

void Task2Ms(void *data)
{
    char task = *(char *)data;
    OS_TCB MYTaskData;
    INT8U err;
    int count = 0;
    int tick = 0;
    struct exynos5_gpio_part2 *gpio2;

    gpio2 = (struct exynos5_gpio_part2 *) EXYNOS5_GPIO_PART2_BASE;

    for (;;) {
        OSSemPend(Tick2MsSem, 0, &err);
        s5p_gpio_set_value(&gpio2->h1, 2, 1);
        OSTimeDly(2);
        OSSemPost(Tick4MsSem);
        s5p_gpio_set_value(&gpio2->h1, 2, 0);
    }
}

void Task4Ms(void *data)
{
    char task = *(char *)data;
    OS_TCB MYTaskData;
    INT8U err;
    struct exynos5_gpio_part2 *gpio2;
    gpio2 = (struct exynos5_gpio_part2 *) EXYNOS5_GPIO_PART2_BASE;

    for (;;) {
        OSSemPend(Tick4MsSem, 0, &err);
        s5p_gpio_set_value(&gpio2->h1, 3, 1);
        OSTimeDly(4);
        OSSemPost(Tick8MsSem);
        s5p_gpio_set_value(&gpio2->h1, 3, 0);
    }
}

void Task8Ms(void *data)
{
    char task = *(char *)data;
    INT8U err;

    struct exynos5_gpio_part1 *gpio1;
    gpio1 = (struct exynos5_gpio_part1 *) EXYNOS5_GPIO_PART1_BASE;


    for (;;) {
        OSSemPend(Tick8MsSem, 0, &err);
        s5p_gpio_set_value(&gpio1->d1, 5, 1);
        OSTimeDly(8);
        OSSemPost(Tick1MsSem);
        s5p_gpio_set_value(&gpio1->d1, 5, 0);
    }
}

void TaskStart(void *data)
{
    int i;
    printf("TaskStart\n");
    init_time();
    EnableTimer();
    struct exynos5_gpio_part2 *gpio2;
    struct exynos5_gpio_part1 *gpio1;

    gpio2 = (struct exynos5_gpio_part2 *) EXYNOS5_GPIO_PART2_BASE;
    gpio1 = (struct exynos5_gpio_part1 *) EXYNOS5_GPIO_PART1_BASE;


    Tick1MsSem = OSSemCreate(0);
    Tick2MsSem = OSSemCreate(0);
    Tick4MsSem = OSSemCreate(0);
    Tick8MsSem = OSSemCreate(0);
    s5p_gpio_direction_output(&gpio2->h1, 1, 0);
    s5p_gpio_direction_output(&gpio2->h1, 2, 0);
    s5p_gpio_direction_output(&gpio2->h1, 3, 0);
    s5p_gpio_direction_output(&gpio1->d1, 5, 0);

    OSStatInit(); /* Initialize uC/OS-II's statistics */

    for (i = 0; i < N_TASKS; i++) {
        TaskData[i] = i;
    }

    OSTaskCreate(Task1Ms, (void *) &TaskData[0],
            &TaskStk[0][TASK_STK_SIZE - 1], 11 + 0);
    OSTaskCreate(Task2Ms, (void *) &TaskData[1],
            &TaskStk[1][TASK_STK_SIZE - 1], 11 + 1);
    OSTaskCreate(Task4Ms, (void *) &TaskData[2],
            &TaskStk[2][TASK_STK_SIZE - 1], 11 + 2);
    OSTaskCreate(Task8Ms, (void *) &TaskData[3],
            &TaskStk[3][TASK_STK_SIZE - 1], 11 + 3);

    printf("OSStatInit\n");

    while (1) {
        OSCtxSwCtr = 0;
#if 0
        for(i = 0; i <N_TASKS; i++){
            OSTaskQuery(11+i, &(MYTaskData[i]));
        }

        printf("---------------------------------\n");
        printf("   # 1Task \tstatus \tPrio \n");
        printf("---------------------------------\n");
        for(i = 0; i < N_TASKS; i++){
            printf("   Task%d() : \t%s \t%d\n", i, "READY", MYTaskData[i].OSTCBPrio);
        }
        printf("---------------------------------\n");
        printf("#Tasks :%5d\tCPU Usage :%3d%\n", OSTaskCtr, OSCPUUsage);
        printf("#Task switch/sec :%5d\n", OSCtxSwCtr);
        printf("\n");
#endif
        OSTimeDlyHMSM(0, 0, 5, 0);
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

