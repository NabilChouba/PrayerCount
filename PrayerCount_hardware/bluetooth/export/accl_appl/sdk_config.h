/* 
 *  Copyright (C) 2009 MindTree Ltd.
 *  All rights reserved.
 */

/**
 *  @file sdk_config.h
 *
 *  This header file contains configuration parameters defined by the
 *  application developer
 */

#ifndef _H_SDK_CONFIG_
#define _H_SDK_CONFIG_

/* ----------------------------------------------- Header File Inclusion */

/* ----------------------------------------------- Common Config Parameters */

/**
 * Bluetooth Protocol Related Configuration Parameters
 */
/* Local device name */
//#define SDK_CONFIG_DEVICE_NAME "BlueMSP-Demo"
#define SDK_CONFIG_DEVICE_NAME "BlueMSP-Demo"
/* Name of the Remote BT Device to which SPP connection has to be established */
#define SDK_REM_DEV_NAME_PREFIX     "BlueMSP-Demo"

/* Max length of Remote device name */
#define SDK_REM_BT_DEV_NAME_MAX_LEN  17

/**
 * Default PIN Code, this pin code is used if authentication procedure takes
 * place with a remote BT device that doesn't support SSP (Secure Simple 
 * Pairing), but if both devices support SSP then this parameter is ignored.
 */
#define SDK_CONFIG_PIN "0000"

/* ACL packet type to be used while initiating connection */
#define SDK_CONFIG_ACL_PKT_TYPE (LMP_ACL_DM1|LMP_ACL_DH1|LMP_ACL_DM3|\
                                 LMP_ACL_DH3|LMP_ACL_DM5|LMP_ACL_DH5)

/* Bluetooth ACL Link Supervision Timeout */
#define SDK_CONFIG_LINK_SUPERVISION_TIMEOUT 0x1900

/* Bluetooth Link Policy Settings */
#define SDK_CONFIG_LINK_POLICY_SETTINGS  0x0005

/* HCI Inquiry Parameters */
#define SDK_INQUIRY_LAP      BT_LIAC
#define SDK_INQUIRY_LEN      0x07
#define SDK_NUM_RESPONSES    0x00

/**
 * HCI Inquiry Scan Parameter, additional inquiry scan access codes can be 
 * included in the list by adding a comma and the access code.
 */
#define SDK_INQUIRY_SCAN_LAP     BT_GIAC, BT_LIAC

/* Maximum number of devices discovered during Inquiry */
#define SDK_INQ_MAX_DEVICES      10

/* SDK Class of Device Field */
#define SDK_CONFIG_COD           BT_MSC_LIM_DISC_MODE|BT_MDC_TOY|BT_TMC_GAME

/* SDK Input Output Capability */
/* Note: The default value (No Input No Output) uses "Just Works" association  
 * model for authentication. This parameter is applicable only for BT 2.1 
 * scenarios.
 */
#define SDK_IO_CAPABILITY        SM_IO_CAPABILITY_NO_INPUT_NO_OUTPUT

/* Flag to enable sniff mode */
#define SDK_ENABLE_SNIFF_MODE

/* Minimum sniff interval to be used */
#define SDK_CONFIG_SNIFF_MIN_INTERVAL 0x00A0
/* Maximum sniff interval to be used */
#define SDK_CONFIG_SNIFF_MAX_INTERVAL 0x00A0
/* Sniff attempt */
#define SDK_CONFIG_SNIFF_ATTEMPT 8
/* Sniff timeout value */
#define SDK_CONFIG_SNIFF_TIMEOUT 1

/* Enables EHCILL mode */
#define SDK_EHCILL_MODE

/* Inactivity_timeout (x * 1.25 milli secs) */
#define SDK_EHCILL_INACTIVITY_TIMEOUT       0x0960
/* Retransmit_timeout (x * 1.25 milli secs) */
#define SDK_EHCILL_RETRANSMISSION_TIMEOUT   0x0190
/* Rts pulse width (x micro secs) */
#define SDK_EHCILL_RTS_PULSE_WIDTH          0x96

/* Enables TI BT Low Power Scan */
#define SDK_LPS_MODE

/* Number of scans required in order to get back into LPS */
#define SDK_LPS_DISABLE_SWEEPS_LENGTH       0x012C
/* The number of consequent LPS scans that trigger regular scan */
#define SDK_LPS_POSITIVE_SWEEPS_TH          0x0004
/* Minimum time between LPS scans in frames (1.25 milli secs) */
#define SDK_LPS_MIN_SCAN_INTERVAL           0x0A


/* Setting MAX OUTPUT POWER LEVEL */
#define SDK_MAX_OUTPUT_POWER_LEVEL  OP_POWER_6_0

/* 
 * Port address of peer SPP de-vice, to be used only if the ap-plication needs
 * to initiate con-nection with a fixed SPP de-vice on this fixed port.
 * It is not necessary that if SDK_CONFIG_SPP_PEER_BD_ADDR is defined
 * then this configuration parameter is also defined.
 */
#define SDK_CONFIG_SPP_PEER_SERVER_CHANNEL 0x01

/* SPP Service Authentication level */
#define SDK_CONFIG_SPP_AUTHENTICATION_LEVEL  SM_SERVICE_SECURITY_LEVEL_2
/* SPP Service Authorization Requirements */
#define SDK_CONFIG_SPP_AUTHORIZATION_REQ     SM_SERVICE_AUTHORIZATION_NONE
/* SPP Service Bonding Requirements */
#define SDK_CONFIG_SPP_BONDING_REQ           0x00

/**
 * EtherMind BT Stack Related Configuration Parameters
 */
/* SDP query buffer length for SPP */
#define SDK_SPP_ATTRIB_DATA_LEN 148

/**
 * UART Related Configuration Parameters
 */
/* Definitions to select the baudrate */
#define BAUDRATE_9600       9600    /* Not supported */
#define BAUDRATE_19200      19200   /* Not supported */
#define BAUDRATE_38400      38400   /* Not supported */
#define BAUDRATE_57600      57600   /* Not supported */
#define BAUDRATE_115200     115200  /* Supported @ 8MHz, 12MHz, 18MHz, 25MHz */
#define BAUDRATE_230400     230400  /* Not supported */
#define BAUDRATE_460800     460800  /* Not supported */
#define BAUDRATE_921600     921600  /* Supported @ 25MHz */

/* Note : The default baud rate for all frequencies is 115200 bps.
 * 921600 bps is supported only with 25MHz CPU clock.*/
#define BT_UART_CONFIG_BAUDRATE  BAUDRATE_115200

/* 
 * UART parameters, the PIOs which are mapped to Rx, Tx, RTS, CTS,
 * ACLK (32.768kHz slow clock), nShutdown and "flight mode" related
 * Pin Assignments for CC2560
 */
/* Bluetooth UART port definitions */
/* Select UART Tx and Rx port selction register */
#define BT_UART_PORT_SEL 	P9SEL
/* select UART Tx and Rx port direction register */
#define BT_UART_PORT_DIR 	P9DIR
/* select UART Tx and Rx port OUTregister */
#define BT_UART_PORT_OUT  	P9OUT
/* UART tx pin */
#define BT_UART_TX_PIN 		BIT4
/* UART rx pin */
#define BT_UART_RX_PIN 		BIT5
/* UART RTS port direction selection register */
#define BT_UART_RTS_PORT_DIR 	P1DIR
/* UART CTS port direction selection register */
#define BT_UART_CTS_PORT_DIR 	P1DIR
/* UART CTS pin */
#define BT_UART_CTS_PIN 	BIT4 //Rev 1.1
/* UART RTS pin */
#define BT_UART_RTS_PIN 	BIT3 //Rev 1.1
/* UART CTS PORT IES */
#define BT_UART_CTS_PORT_IES	P1IES
/* UART CTS PROT IFG */
#define BT_UART_CTS_PORT_IFG	P1IFG
/* UART VTS PORT IE */
#define BT_UART_CTS_PORT_IE	P1IE
/* UART CTS PORT SELECT */
#define BT_UART_CTS_PORT_SEL	P1SEL
/* UART RTS PORT SELECT */
#define BT_UART_RTS_PORT_SEL	P1SEL
/* UART RTS port output register */
#define BT_UART_RTS_PORT_OUT 	P1OUT
/* UART CTS port output register */
#define BT_UART_CTS_PORT_OUT 	P1OUT
/* UART CTS port input register */
#define BT_UART_CTS_PORT_IN 	P1IN
/* UART CTS Port Resistor Enable */
#define BT_UART_CTS_PORT_REN	P1REN
/* UART control register 0 */
#define BT_UART_REG_UCAXCTL0 	UCA2CTL0
/* UART control register 1 */
#define BT_UART_REG_UCAXCTL1 	UCA2CTL1
/* UART baudrate selecation register 0 */
#define BT_UART_REG_UCAXBR0 	UCA2BR0
/* UART baudrate selecation register 1 */
#define BT_UART_REG_UCAXBR1 	UCA2BR1
/* UART Interrupt vector register */
#define BT_UART_REG_UCAXIV 	UCA2IV
/* UART output status register */
#define BT_UART_REG_UCAXSTAT 	UCA2STAT
/* UART Tx buffer register */
#define BT_UART_REG_UCAXTXBUF 	UCA2TXBUF
/* UART Rx buffer register */
#define BT_UART_REG_UCAXRXBUF 	UCA2RXBUF
/* UART modulation control register */
#define BT_UART_REG_UCAXMCTL 	UCA2MCTL
/* UART interrupt enable register */
#define BT_UART_REG_UCAXIE 	UCA2IE
/* UART interrupt flag register */
#define BT_UART_REG_UCAXIFG 	UCA2IFG
#define BT_UART_VECTOR          USCI_A2_VECTOR

/**
 * BT controller Related Configuration Parameters
 */
/* BT controller Clock Port Select */
#define BT_RF_CLK_PORT_SEL		P2SEL
/* BT controller Clock Port Pin */
#define BT_RF_CLK_PORT_PIN		BIT6
/* BT controller nShutdown Port Direction */
#define BT_RF_NSHUTDOWN_PORT_DIR	P2DIR
/* BT controller nShutdown Port Pin */
#define BT_RF_NSHUTDOWN_PORT_PIN	BIT7
/* BT controller nShutdown Port Out */
#define BT_RF_NSHUTDOWN_PORT_OUT	P2OUT

/**
 * MSP430 Related Configuration Parameters
 */
/* Define the system clock for MSP430 */
/* Supported CPU clocks: 8MHz, 12MHz, 18MHz, 25MHz */
/* Note 1: 25MHz is not supported on F5438 device */
/* Note 2: See baudrate section above for supported baudrate-CPU combinations */
#define SYSTEM_CLK SYSCLK_18MHZ

/* Define the inactivity period for MSP430 to enter LPM */
#define INACTIVITY_TIMEOUT	30

/* Enable MSP430 LPM feature */
#define MSP430_LPM_ENABLE

/* Define the low power mode for MSP430 */
#define SDK_MSP430_LPM	MSP430_LPM_3
#endif /* _H_SDK_CONFIG_ */
