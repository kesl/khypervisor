/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#ifndef __ASM_ARCH_UART_H_
#define __ASM_ARCH_UART_H_

#include "arch_types.h"

/* baudrate rest value */
union br_rest {
        unsigned short        slot;                /* udivslot */
        unsigned char        value;                /* ufracval */
};

struct s5p_uart {
        unsigned int        ulcon;
        unsigned int        ucon;
        unsigned int        ufcon;
        unsigned int        umcon;
        unsigned int        utrstat;
        unsigned int        uerstat;
        unsigned int        ufstat;
        unsigned int        umstat;
        unsigned char        utxh;
        unsigned char        res1[3];
        unsigned char        urxh;
        unsigned char        res2[3];
        unsigned int        ubrdiv;
        union br_rest        rest;
        unsigned char        res3[0xffd0];
};

/* Exynos 5250 UART register macros */
#define UTXH        0x20
#define UFSTAT      0x18
/* UART Base Address determined by Hypervisor's Stage 2 Translation Table */
#define UART_BASE           0x12C20000

int serial_tst_fifo(void);
int serial_getc(void);
void serial_putc(const char c);
void serial_setbrg_dev(uint32_t base);
void serial_init(void);
#endif

