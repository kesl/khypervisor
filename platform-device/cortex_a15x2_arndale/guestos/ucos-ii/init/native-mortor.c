#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#if 0
#include "gfxdev.h"
#endif

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

enum comport_number { COM1, COM2, COM3, COM4 };

/* baudrate setting */
#define BAUDRATE B38400
/* POSIX compliant source */
#define _POSIX_SOURCE 1

fd_set readfds;
struct    timeval tv;

/* for terminal attribute save & setting */
struct termios oldtio, newtio;
/* serial port file descriptor */
int serial_fd;

void gprintf(char *fmt, ...)
{
    char buf[512];
    int ret;
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    ret = write(serial_fd, (void *)buf, strlen(buf));
    if (ret < 0)
        printf("serial write error: %s\n", strerror(errno));
}

int getch(int type)
{
    int read_bytes;
    unsigned char buf;

    int state;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_SET(serial_fd, &readfds);
    state = select(serial_fd+1, &readfds, (fd_set *)0, (fd_set *)0, &tv);

    switch (state) {
    case -1:
        printf("selct errot\n");
        break;
    case 0:
        printf("Time over\n");
        return type;
    default:
        read_bytes = read(serial_fd, &buf, 1);
        if (read_bytes > 0)
            return (int)buf;
        if ((read_bytes < 0) && (errno != EINTR) && (errno != EAGAIN)) {
            printf("getch error\n");
            return -1;
        }
        break;
    }

    /*

       read_bytes = read(serial_fd, &buf, 1);
       if ( read_bytes > 0 )
       return (int)buf;
       if ( (read_bytes < 0) && (errno != EINTR) && (errno != EAGAIN) ) {
       printf("getch error\n");
       return -1;
       }
     */
}


unsigned char UART0_getc(int type)
{
    return (unsigned char)getch(type);
}
void UART0_putc(unsigned char data)
{
    int ret;

    ret = write(serial_fd, (void *)&data, 1);
    if (ret < 0)
        printf("putc error\n");

}

void Ack(void)
{
    /*
       volatile int cnt = 0;
       volatile int cnt2 = 0;
       while(UART0_getc(0xAC) != 0xAC){
       cnt++;
       if(cnt > 10000){
       cnt2++;
       cnt = 0;
       if(cnt2 > 10000){
       printf("fucking break!\n");
       break;
       }
       }
       }
     */
}

void WaitReport(void)
{
    /*
       volatile int cnt = 0;
       volatile int cnt2 = 0;
       while(UART0_getc(0xED) != 0xED){
       cnt++;
       if(cnt > 10000){
       cnt2++;
       cnt = 0;
       if(cnt2 > 10000){
       printf("fucking break!\n");
       break;
       }
       }
       }
     */
}

void DegSet(unsigned long did, unsigned long sid, unsigned long acc,
        unsigned long degree)
{
    UART0_putc((char)0x5);
    UART0_putc((char)0);
    UART0_putc((char)MSG_DegSet);
    UART0_putc((char)sid);
    UART0_putc((char)acc);
    UART0_putc((char)degree);
    Ack();
}

void MemSet(unsigned long did)
{
    UART0_putc((char)0x2);
    UART0_putc((char)0);
    UART0_putc((char)MSG_MemSet);
    Ack();
}

void Report(unsigned long did)
{
    UART0_putc((char)0x2);
    UART0_putc((char)0);
    UART0_putc((char)MSG_Report);
    Ack();
}


int serial_open(int which_port)
{
    char *port_str;

    switch (which_port) {
    case COM1:
        port_str = "/dev/ttySAC2";
        break;
    case COM2:
        port_str = "/dev/ttyS1";
        break;
    case COM3:
        port_str = "/dev/ttyS2";
        break;
    case COM4:
        port_str = "/dev/ttyS3";
        break;
    default:
        printf("input serial port error\n");
        exit(EXIT_FAILURE);
    }

    serial_fd = open(port_str, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_fd < 0) {
        printf("serial_port open error: %s\n", port_str);
        exit(-1);
    }

    tcgetattr(serial_fd, &oldtio);

    bzero(&newtio, sizeof(newtio));
    /* newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD; */
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;

#if 0  /* raw output */
    newtio.c_oflag &= ~OPOST;
#else
    /* post processing enable */
    newtio.c_oflag |= OPOST;
    newtio.c_oflag |= ONLCR;
#endif

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;
    newtio.c_cc[VMIN]     = 1;

    tcflush(serial_fd, TCIFLUSH);
    tcsetattr(serial_fd, TCSANOW, &newtio);

    return 0;
}

void serial_close(void)
{
    tcsetattr(serial_fd, TCSANOW, &oldtio);
    close(serial_fd);
}

int thread_arr[8];
int thread_id;


pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lock[8]  = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
};
pthread_cond_t   thread_cond[8] = {
    PTHREAD_COND_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
};

volatile int _i;
void my_delay(int x)
{
    for (_i = 0 ; _i < x; _i++)
        ;
}

void *control_thread(void *pdata)

{
    unsigned long did, sid, stage;
    unsigned long x, y;
    unsigned char err;
    int thread_id = *(int *)pdata;

    did = 0;
    sid = 0;
    stage = 0;


    while (1) {
        pthread_mutex_lock(&my_mutex);
        if (!stage) {
#ifndef DISALBE_CONTROL
            DegSet(did, sid, 1, 179);
            MemSet(did);
            Report(did);
            WaitReport();
#endif
        } else {
#ifndef DISALBE_CONTROL
            DegSet(did, sid, 1, 0);
            MemSet(did);
            Report(did);
            WaitReport();
#endif
        }

        sid = (sid + 1) % 8;
        if (!sid)
            stage = (stage + 1) & 0x1;
        my_delay(500000);
        usleep(200000);

        pthread_mutex_unlock(&my_mutex);
    }
}

void MotorAbsTurn(unsigned long id, unsigned long angle)
{
    unsigned char err;
    pthread_mutex_lock(&my_mutex);
    /*
       DegSet(0, id, 1, angle);
       MemSet(0);
       Report(0);
       WaitReport();
     */
    DegSet(0, id, 1, angle);
    Report(0);
    WaitReport();
    pthread_mutex_unlock(&my_mutex);
}

void *MotorFlagControlTask(void *data)
{
    unsigned long sid, stage;
    sid = 0;
    stage = 0;

    for (;;) {
        if (!stage)
            MotorAbsTurn(sid, 120);
        else
            MotorAbsTurn(sid, 0);
        sid = (sid + 1) % 6;
        if (!sid)
            stage = (stage + 1) & 0x1;
        my_delay(3000000);
        usleep(100 * 1000);
    }
}

void *MotorRailControlTask(void *data)
{
    unsigned long sid, stage;
    sid = 6;
    stage = 0;

    for (;;) {
        if (!stage) {
            MotorAbsTurn(sid, 180);
            stage = 1;
        } else {
            MotorAbsTurn(sid, 0);
            stage = 0;
        }
        my_delay(3000000);
        usleep(1000000);
    }
}

void *MotorLockControlTask(void *data)
{
    unsigned long sid, stage;
    sid = 7;
    stage = 0;

    for (;;) {
        if (!stage) {
            MotorAbsTurn(sid, 180);
            stage = 1;
        } else {
            MotorAbsTurn(sid, 0);
            stage = 0;
        }
        my_delay(3000000);
        usleep(1000000);
    }
}

#define N_TASKS   8

int main(void)
{
    int result = 0;
    int i = 0;
    pthread_t p_thread;
    int policy = SCHED_OTHER;
    int min_prio_for_policy = 0;

    serial_open(COM1);
    pthread_create(&p_thread, NULL, MotorLockControlTask, &i);
    pthread_create(&p_thread, NULL, MotorRailControlTask, &i);
    pthread_create(&p_thread, NULL, MotorFlagControlTask, &i);

    pthread_attr_t thAttr;
    pthread_attr_init(&thAttr);
    pthread_attr_getschedpolicy(&thAttr, &policy);
    min_prio_for_policy = sched_get_priority_min(policy);

    pthread_setschedprio(p_thread, min_prio_for_policy);
    pthread_attr_destroy(&thAttr);

    while (1)
        sleep(5);
    printf("Do not access here!\n");
    serial_close();
    return 0;
}
