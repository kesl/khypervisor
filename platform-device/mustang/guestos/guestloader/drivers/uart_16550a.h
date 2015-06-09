#ifndef __UART_16550A__
#define __UART_16550A__

enum baud_rate {
    baud_115200 =0,
    baud_57600,
    baud_38400,
    baud_19200,
    baud_9600,
};
#define UART_16550A_BASE 0x1C020000
/* UART_LCR.DLAB = 0 */
#define UART_RBR 0x00   /* Receiver Buffer Register */
#define UART_THR 0x00   /* Transmitter Holding Register */
/* UART_LCR.DLAB = 1 */
#define UART_DLL 0x00   /* Divisor Latch LSB */
#define UART_DLM 0x01   /* Divisor Latch MSB */
#define UART_IER 0x01   /* Interrupt Enable Register */
#define UART_IIR 0x02   /* Interrupt Identification Register */
#define UART_LCR 0x03   /* Line Control Register RO */
#define UART_FCR 0x02   /* FIFO Control Register WO */
#define UART_MCR 0x04   /* Modem Control Register */
#define UART_LSR 0x05   /* Line Status Register */
#define UART_MSR 0x06   /* Modem Status Register */
#define UART_SCR 0x07   /* Scratch Register */

/* MCR */
#define UART_MCR_DTR    0x01
#define UART_MCR_RTS    0x02

/* IER */
#define UART_IER_RDA    (0x1 << 0)   /* Received Data Available */
#define UART_IER_THRE   (0x1 << 1)  /* Transmitter Holding Register Empty */
#define UART_IER_RLSRC  (0x1 << 2) /* Receiver Line Status Register Change */
#define UART_IER_MSRC   (0x1 << 3) /* Modem Status Register Change */

/* IIR */
#define UART_IIR_NOINT (1 << 0) /* NO INTerrupt pendinfg */
//#define UART_IIR

/* FCR */
#define UART_FCR_ENABLE     (0x1 << 0)
#define UART_FCR_DISABLE    (0X0 << 0)
#define UART_FCR_CLR_RX   (0x1 << 1)
#define UART_FCR_CLR_TX   (0x1 << 2)
#define UART_FCR_DMA_MODE_0 (0x1 << 3)
#define UART_FCR_DMA_MODE_1 (0x1 << 3)
#define UART_FCR_IRQ_TRG_LV_1   (0x0 << 6)
#define UART_FCR_IRQ_TRG_LV_4   (0x1 << 6)
#define UART_FCR_IRQ_TRG_LV_8   (0x2 << 6)
#define UART_FCR_IRQ_TRG_LV_14  (0x3 << 6)

/* LCR  Options */

/* LCR Data Word Length */
#define UART_LCR_DWL_5  0x00    /* 5bits */
#define UART_LCR_DWL_6  0X01    /* 6bits */
#define UART_LCR_DWL_7  0X02    /* 7bits */
#define UART_LCR_DWL_8  0x03    /* 8bits */

/* LCR Stop Bits */
#define UART_LCR_SB_1   (0x0 << 2)  /* 1 stop bit */
#define UART_LCR_SB_2   (0x1 << 2)  /* 1.5 stop bits & 2 stop bits */

/* LCR Parity */
#define UART_LCR_NP (0x00 << 3)  /* No Parity */
#define UART_LCR_OP (0x01 << 3) /* Odd Parity */
#define UART_LCR_EP (0x03 << 3) /* Even Parity */
#define UART_LCR_HP (0x05 << 3) /* High Parity */
#define UART_LCR_LP (0x07 << 3) /* Low Parity */

/* LCR Break Signal */
#define UART_LCR_BSD    (0x0 << 6)  /* Break Signal Disalbe */
#define UART_LCR_BSE    (0x1 << 6)  /* Break Signal Enable */

/* LCR DLAB */
#define UART_LCR_DLAB_0 (0x0 << 7)  /* DLAB : RBR, THR and IER accessible */
#define UART_LCR_DLAB_1 (0x1 << 7)  /* DLAB : DLL and DLM accessible */

/* MCR */

/* LSR */
#define UART_LSR_DA (0x1 << 0) /* Data Avaiable */
#define UART_LSR_OE (0x1 << 1) /* Overrun Error */
#define UART_LSR_PE (0x1 << 2) /* Parity Error */
#define UART_LSR_FE (0x1 << 3) /* Framing Error */
#define UART_LSR_BSR (0x1 << 4) /* Break Signal Received */
#define UART_LSR_THE (0x1 << 5) /* THR is Empty */
#define UART_LSR_THEI (0x1 << 6) /* THR is Empty and line is Idle */
#define UART_LSR_EDF (0x1 << 7) /* Errornous Data in Fifo */

int uart_16550a_tst_fifo();
void uart_16550a_putc (const char);
void uart_16550a_init (unsigned int, enum baud_rate);
char uart_16550a_getc ();
void uart_16550a_print_hex(uint32_t);
void uart_16550a_print_hex32(uint32_t);
void uart_16550a_print_hex64(uint64_t);
#endif
