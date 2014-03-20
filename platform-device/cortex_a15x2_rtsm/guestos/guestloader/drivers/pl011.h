#ifndef __PL011_H__
#define __PL011_H__
#include "arch_types.h"

#define PL011_BASE       0x1C090000

/*
 * ARM PrimeCell UART's (PL011)
 * ------------------------------------
 *
 */
/*  Data read or written from the interface. */
#define PL011_UARTDR                   0x00
/*  Receive status register (Read). */
#define PL011_UARTRSR                  0x04
/*  Error clear register (Write). */
#define PL011_UARTECR                  0x04
/*  Flag register (Read only). */
#define PL011_UARTFR                   0x18
#define PL011_UARTRSR_OE               0x08
#define PL011_UARTRSR_BE               0x04
#define PL011_UARTRSR_PE               0x02
#define PL011_UARTRSR_FE               0x01

#define PL011_UARTFR_TXFE              0x80
#define PL011_UARTFR_RXFF              0x40
#define PL011_UARTFR_TXFF              0x20
#define PL011_UARTFR_RXFE              0x10
#define PL011_UARTFR_BUSY              0x08
#define PL011_UARTFR_TMSK    (PL011_UARTFR_TXFF + PL011_UARTFR_BUSY)

#define PL011_UARTIBRD                 0x24
#define PL011_UARTFBRD                 0x28
#define PL011_UARTLCR_H                0x2C
#define PL011_UARTCR                   0x30
#define PL011_UARTIMSC                 0x38
#define PL011_UARTPERIPH_ID0           0xFE0

#define PL011_UARTLCR_H_SPS             (1 << 7)
#define PL011_UARTLCR_H_WLEN_8          (3 << 5)
#define PL011_UARTLCR_H_WLEN_7          (2 << 5)
#define PL011_UARTLCR_H_WLEN_6          (1 << 5)
#define PL011_UARTLCR_H_WLEN_5          (0 << 5)
#define PL011_UARTLCR_H_FEN             (1 << 4)
#define PL011_UARTLCR_H_STP2            (1 << 3)
#define PL011_UARTLCR_H_EPS             (1 << 2)
#define PL011_UARTLCR_H_PEN             (1 << 1)
#define PL011_UARTLCR_H_BRK             (1 << 0)

#define PL011_UARTCR_CTSEN             (1 << 15)
#define PL011_UARTCR_RTSEN             (1 << 14)
#define PL011_UARTCR_OUT2              (1 << 13)
#define PL011_UARTCR_OUT1              (1 << 12)
#define PL011_UARTCR_RTS               (1 << 11)
#define PL011_UARTCR_DTR               (1 << 10)
#define PL011_UARTCR_RXE               (1 << 9)
#define PL011_UARTCR_TXE               (1 << 8)
#define PL011_UARTCR_LPE               (1 << 7)
#define PL011_UARTCR_IIRLP             (1 << 2)
#define PL011_UARTCR_SIREN             (1 << 1)
#define PL011_UARTCR_UARTEN            (1 << 0)

#define PL011_UARTIMSC_OEIM            (1 << 10)
#define PL011_UARTIMSC_BEIM            (1 << 9)
#define PL011_UARTIMSC_PEIM            (1 << 8)
#define PL011_UARTIMSC_FEIM            (1 << 7)
#define PL011_UARTIMSC_RTIM            (1 << 6)
#define PL011_UARTIMSC_TXIM            (1 << 5)
#define PL011_UARTIMSC_RXIM            (1 << 4)
#define PL011_UARTIMSC_DSRMIM          (1 << 3)
#define PL011_UARTIMSC_DCDMIM          (1 << 2)
#define PL011_UARTIMSC_CTSMIM          (1 << 1)
#define PL011_UARTIMSC_RIMIM           (1 << 0)

/** @brief Return status whether FIFO is empty or not
 *  @param base pl011's base address.
 *  @return There is not a data in the FIFO then returns 0, otherwise 1.
 */
int pl011_tst_fifo(uint32_t base);

/** @brief Like putc
 *  @param c Character to be written.
 */
void pl011_putc(const char c);

/** @brief Like getc
 *  @param base pl011's base address.
 *  @return Read from pl011's FIFO.
 */
char pl011_getc(uint32_t base);

/** @brief Initializes pl011.
 *  @param base pl011's base address.
 *  @param baudrate To set baud rate.
 *  @param input_clock pl011's clock.
 */
void pl011_init(uint32_t base, uint32_t baudrate, uint32_t input_clock);
#endif
