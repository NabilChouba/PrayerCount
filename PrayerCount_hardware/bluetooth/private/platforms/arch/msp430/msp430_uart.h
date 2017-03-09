
/**
 * Copyright (c) 2009 MindTree Ltd. All rights reserved.
 * \file    msp430_uart.h
 * \brief   Contains the MSP430 UART related definitions
 */

#ifndef _H_MSP430_UART_
#define _H_MSP430_UART_

/**
 * Tool Chain Configuration Parameter
 */
/* Define this flag if using IAR Compiler */
#define TOOLCHAIN_IAR

/* definitions for MSP430 UART clock */
#define UART_CLK_32KHz      32000
#define UART_CLK_1MHz       1000000
#define UART_CLK_2MHz       2000000
#define UART_CLK_4MHz       4000000
#define UART_CLK_8MHz       8000000
#define UART_CLK_12MHz      12000000
#define UART_CLK_16MHz      16000000
#define UART_CLK_20MHz      20000000
#define UART_CLK_25MHz      25000000

/* Define the UART Transport transmit and receive buffer sizes */
#define UART_TX_BUFFER_SIZE 512
#define UART_RX_BUFFER_SIZE 512


#define MAX_CIRCULAR_BUF_SIZE       UART_RX_BUFFER_SIZE
#define MAX_PAYLOAD_LENGTH          260
#define MAX_DATA_RX_PACKETS         10
#define MAX_PKT_HDR_LEN             5

#define HCI_EVENT_PACKET_HEADER_LEN	    3
#define HCI_ACL_DATA_PACKET_HEADER_LEN      5

typedef struct {
    UINT16 start_idx;
    UINT16 length;
    UCHAR pkt_type;
} DATA_RX_QUEUE;

/* ----------------------------------------------- typedefs */
/* MSP430 UART parameter structure */
#ifdef TOOLCHAIN_IAR
typedef struct {
    volatile UINT8 *uart_port_sel;
    volatile UINT8 *uart_port_dir;
    volatile UINT8 *uart_port_out;
    UINT16 uart_tx_pin;
    UINT16 uart_rx_pin;
    volatile UINT8 *uart_rts_port_dir;
    volatile UINT8 *uart_cts_port_dir;
    UINT16 uart_rts_pin;
    UINT16 uart_cts_pin;
    volatile UINT8 *uart_rts_port_out;
    volatile UINT8 *uart_cts_port_out;
    volatile UINT8 *uart_rts_port_in;
    unsigned char const volatile __data16 *uart_cts_port_in;
    volatile UINT8 *uart_cts_port_ren;
    volatile UINT8 *uart_cts_port_sel;
    volatile UINT8 *uart_rts_port_sel;
    volatile UINT8 *uart_cts_ies;
    volatile UINT8 *uart_cts_ifg;
    volatile UINT8 *uart_cts_ie;
    volatile UINT8 *uart_reg_ucaxctl0;
    volatile UINT8 *uart_reg_ucaxctl1;
    volatile UINT8 *uart_reg_ucaxbr0;
    volatile UINT8 *uart_reg_ucaxbr1;
    unsigned short volatile __data16 *uart_reg_ucaxiv;
    volatile UINT8 *uart_reg_ucaxstat;
    volatile UINT8 *uart_reg_ucaxtxbuf;
    unsigned char const volatile __data16 *uart_reg_ucaxrxbuf;
    volatile UINT8 *uart_reg_ucaxmctl;
    volatile UINT8 *uart_reg_ucaxie;
    volatile UINT8 *uart_reg_ucaxifg;
    UINT32 uart_baudrate;
    UINT32 config_baudrate;
} UART_CONFIG_PARAMS;
#else /* ccsv4 */
typedef struct {
    volatile UINT8 *uart_port_sel;
    volatile UINT8 *uart_port_dir;
    volatile UINT8 *uart_port_out;
    UINT16 uart_tx_pin;
    UINT16 uart_rx_pin;
    volatile UINT8 *uart_rts_port_dir;
    volatile UINT8 *uart_cts_port_dir;
    UINT16 uart_rts_pin;
    UINT16 uart_cts_pin;
    volatile UINT8 *uart_rts_port_out;
    volatile UINT8 *uart_cts_port_out;
    volatile UINT8 *uart_rts_port_in;
    volatile UINT8 *uart_cts_port_in;
    volatile UINT8 *uart_cts_port_ren;
    volatile UINT8 *uart_cts_port_sel;
    volatile UINT8 *uart_rts_port_sel;
    volatile UINT8 *uart_cts_ies;
    volatile UINT8 *uart_cts_ifg;
    volatile UINT8 *uart_cts_ie;
    volatile UINT8 *uart_reg_ucaxctl0;
    volatile UINT8 *uart_reg_ucaxctl1;
    volatile UINT8 *uart_reg_ucaxbr0;
    volatile UINT8 *uart_reg_ucaxbr1;
    volatile unsigned int *uart_reg_ucaxiv;
    volatile UINT8 *uart_reg_ucaxstat;
    volatile UINT8 *uart_reg_ucaxtxbuf;
    volatile UINT8 *uart_reg_ucaxrxbuf;
    volatile UINT8 *uart_reg_ucaxmctl;
    volatile UINT8 *uart_reg_ucaxie;
    volatile UINT8 *uart_reg_ucaxifg;
    UINT32 uart_baudrate;
    UINT32 config_baudrate;
} UART_CONFIG_PARAMS;

#endif /* TOOLCHAIN_IAR */

extern UART_CONFIG_PARAMS bt_uart_config;
#define UART_TRANSMIT(byte) \
{ \
    while (bt_uart_config.uart_cts_pin & *(bt_uart_config.uart_cts_port_in)); \
    *(bt_uart_config.uart_reg_ucaxtxbuf) = (byte); \
}

#define UART_DISABLE_BT_UART_RTS() \
{ \
	 *(bt_uart_config.uart_rts_port_out) |= (bt_uart_config.uart_rts_pin);\
}


/* Enable RTS only if UART RXinterrupt is not pending */
#define UART_ENABLE_BT_UART_RTS() \
{ \
    if ((!(*(bt_uart_config.uart_reg_ucaxifg) & UCRXIFG)) && (!(ADC12IFG)))  {\
	 *(bt_uart_config.uart_rts_port_out) &= ~(bt_uart_config.uart_rts_pin);\
    }\
}

#define UART_ENABLE_BT_UART_TX() \
{ \
	 *(bt_uart_config.uart_reg_ucaxie) |= UCTXIE;\
}

#define UART_DISABLE_BT_UART_TX() \
{ \
	 *(bt_uart_config.uart_reg_ucaxie) &= ~UCTXIE;\
}

#define UART_ENABLE_BT_UART_RX() \
{ \
	 *(bt_uart_config.uart_reg_ucaxie) |= UCRXIE;\
}

#define UART_DISABLE_BT_UART_RX() \
{ \
	 *(bt_uart_config.uart_reg_ucaxie) &= ~UCRXIE;\
}

#endif /* _H_MSP430_UART_ */
