#ifndef __PL01X_H__
#define __PL01X_H__
#include "arch_types.h"

#define PL01X_BASE       0x1C090000
#define PL01X_TYPE_0    0
#define PL01X_TYPE_1    1

/*
 * ARM PrimeCell UART's (PL010 & PL011)
 * ------------------------------------
 *
 *  Definitions common to both PL010 & PL011
 *
 */
/*  Data read or written from the interface. */
#define UART_PL01x_DR                   0x00
/*  Receive status register (Read). */
#define UART_PL01x_RSR                  0x04
/*  Error clear register (Write). */
#define UART_PL01x_ECR                  0x04
/*  Flag register (Read only). */
#define UART_PL01x_FR                   0x18
#define UART_PL01x_RSR_OE               0x08
#define UART_PL01x_RSR_BE               0x04
#define UART_PL01x_RSR_PE               0x02
#define UART_PL01x_RSR_FE               0x01

#define UART_PL01x_FR_TXFE              0x80
#define UART_PL01x_FR_RXFF              0x40
#define UART_PL01x_FR_TXFF              0x20
#define UART_PL01x_FR_RXFE              0x10
#define UART_PL01x_FR_BUSY              0x08
#define UART_PL01x_FR_TMSK    (UART_PL01x_FR_TXFF + UART_PL01x_FR_BUSY)

/*
 *  PL010 definitions
 *
 */
/*  Line control register, high byte. */
#define UART_PL010_LCRH                 0x08
/*  Line control register, middle byte. */
#define UART_PL010_LCRM                 0x0C
/*  Line control register, low byte. */
#define UART_PL010_LCRL                 0x10
/*  Control register. */
#define UART_PL010_CR                   0x14
/*  Interrupt indentification register (Read). */
#define UART_PL010_IIR                  0x1C
/*  Interrupt clear register (Write). */
#define UART_PL010_ICR                  0x1C
/*  IrDA low power counter register. */
#define UART_PL010_ILPR                 0x20

#define UART_PL010_CR_LPE               (1 << 7)
#define UART_PL010_CR_RTIE              (1 << 6)
#define UART_PL010_CR_TIE               (1 << 5)
#define UART_PL010_CR_RIE               (1 << 4)
#define UART_PL010_CR_MSIE              (1 << 3)
#define UART_PL010_CR_IIRLP             (1 << 2)
#define UART_PL010_CR_SIREN             (1 << 1)
#define UART_PL010_CR_UARTEN            (1 << 0)

#define UART_PL010_LCRH_WLEN_8          (3 << 5)
#define UART_PL010_LCRH_WLEN_7          (2 << 5)
#define UART_PL010_LCRH_WLEN_6          (1 << 5)
#define UART_PL010_LCRH_WLEN_5          (0 << 5)
#define UART_PL010_LCRH_FEN             (1 << 4)
#define UART_PL010_LCRH_STP2            (1 << 3)
#define UART_PL010_LCRH_EPS             (1 << 2)
#define UART_PL010_LCRH_PEN             (1 << 1)
#define UART_PL010_LCRH_BRK             (1 << 0)


#define UART_PL010_BAUD_460800            1
#define UART_PL010_BAUD_230400            3
#define UART_PL010_BAUD_115200            7
#define UART_PL010_BAUD_57600             15
#define UART_PL010_BAUD_38400             23
#define UART_PL010_BAUD_19200             47
#define UART_PL010_BAUD_14400             63
#define UART_PL010_BAUD_9600              95
#define UART_PL010_BAUD_4800              191
#define UART_PL010_BAUD_2400              383
#define UART_PL010_BAUD_1200              767
/*
 *  PL011 definitions
 *
 */
#define UART_PL011_IBRD                 0x24
#define UART_PL011_FBRD                 0x28
#define UART_PL011_LCRH                 0x2C
#define UART_PL011_CR                   0x30
#define UART_PL011_IMSC                 0x38
#define UART_PL011_PERIPH_ID0           0xFE0

#define UART_PL011_LCRH_SPS             (1 << 7)
#define UART_PL011_LCRH_WLEN_8          (3 << 5)
#define UART_PL011_LCRH_WLEN_7          (2 << 5)
#define UART_PL011_LCRH_WLEN_6          (1 << 5)
#define UART_PL011_LCRH_WLEN_5          (0 << 5)
#define UART_PL011_LCRH_FEN             (1 << 4)
#define UART_PL011_LCRH_STP2            (1 << 3)
#define UART_PL011_LCRH_EPS             (1 << 2)
#define UART_PL011_LCRH_PEN             (1 << 1)
#define UART_PL011_LCRH_BRK             (1 << 0)

#define UART_PL011_CR_CTSEN             (1 << 15)
#define UART_PL011_CR_RTSEN             (1 << 14)
#define UART_PL011_CR_OUT2              (1 << 13)
#define UART_PL011_CR_OUT1              (1 << 12)
#define UART_PL011_CR_RTS               (1 << 11)
#define UART_PL011_CR_DTR               (1 << 10)
#define UART_PL011_CR_RXE               (1 << 9)
#define UART_PL011_CR_TXE               (1 << 8)
#define UART_PL011_CR_LPE               (1 << 7)
#define UART_PL011_CR_IIRLP             (1 << 2)
#define UART_PL011_CR_SIREN             (1 << 1)
#define UART_PL011_CR_UARTEN            (1 << 0)

#define UART_PL011_IMSC_OEIM            (1 << 10)
#define UART_PL011_IMSC_BEIM            (1 << 9)
#define UART_PL011_IMSC_PEIM            (1 << 8)
#define UART_PL011_IMSC_FEIM            (1 << 7)
#define UART_PL011_IMSC_RTIM            (1 << 6)
#define UART_PL011_IMSC_TXIM            (1 << 5)
#define UART_PL011_IMSC_RXIM            (1 << 4)
#define UART_PL011_IMSC_DSRMIM          (1 << 3)
#define UART_PL011_IMSC_DCDMIM          (1 << 2)
#define UART_PL011_IMSC_CTSMIM          (1 << 1)
#define UART_PL011_IMSC_RIMIM           (1 << 0)

void pl01x_putc(const char c);
char pl01x_getc(uint32_t base);
void pl01x_init(uint32_t base, uint32_t type, uint32_t baudrate,
        uint32_t input_clock);
#endif
