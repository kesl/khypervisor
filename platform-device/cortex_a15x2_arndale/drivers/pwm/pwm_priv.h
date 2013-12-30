#ifndef __PWM_PRIV_H__
#define __PWM_PRIV_H__

#define PWMBASE (void*)0x12DD0000
#define PWM_TIMERREG(x) (PWMBASE + (x))
#define TCFG0   PWM_TIMERREG(0x00)
#define TCFG1   PWM_TIMERREG(0x04)
#define TCON    PWM_TIMERREG(0x08)
#define PWM_TIMERREG2(tmr,reg) PWM_TIMERREG((reg)+0x0c+((tmr)*0x0c))
#define CSTAT PWM_TIMERREG(0x44)
#define TCNTB(tmr)  PWM_TIMERREG2(tmr, 0x00)
#define TCMPB(tmr)  PWM_TIMERREG2(tmr, 0x04)
#define TCNTO(tmr)  PWM_TIMERREG2(tmr, (((tmr) == 4) ? 0x04 : 0x08))
#define TCON_T4RELOAD       (1<<22)
#define TCON_T4MANUALUPD    (1<<21)
#define TCON_T4START        (1<<20)
#define TCON_T0RELOAD       (1<<3)
#define TCON_T0MANUALUPD    (1<<1)
#define TCON_T0START        (1<<0)
#define TCON_T2RELOAD       (1<<15)
#define TCON_T2MANUALUPD    (1<<13)
#define TCON_T2START        (1<<12)
#define TCON_T1MANUALUPD    (1<<9)
#define TCON_T1START        (1<<8)
#define TCON_T1RELOAD       (1<<11)

#endif
           
