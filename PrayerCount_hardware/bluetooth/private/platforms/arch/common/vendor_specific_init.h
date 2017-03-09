
/**
 * Copyright (c) 2009 MindTree Ltd.  All rights reserved.
 * \file    vendor_specific_init.h
 * \brief   This file contains definitions related to vendor specific init script
 */



#ifndef _H_VENDOR_SPECIFIC_INIT_
#define _H_VENDOR_SPECIFIC_INIT_

/* Header File Inclusion */
#include "BT_common.h"
#include "sdk_pl.h"

/* Global Definitions */

/* Structures/Data Types */

/* Macros */
#define V_INIT_FALSE         0
#define V_INIT_IN_PROGRESS   1
#define V_INIT_COMPLETE      2


/* defintions for BT controller vendor specific intialization state machine */
#define V_SET_INIT_SCRIPT_UART_BAUDRATE				3
#define V_SET_INIT_SCRIPT_UART_BAUDRATE_COMPLETE 	4

/* BT Controller vendor specific command opcode to update the uart baudrate */
#define SDK_BT_RF_UPDATE_UART_BAUDRATE_OPCODE        0xFF36
#define SDK_BT_RF_UPDATE_UART_BAUDRATE_COMMAND_LEN   0x04
/* BT controller vendor specific command to configure sleep mode */
#define SDK_BT_RF_SET_SLEEP_MODE                     0xFD0C
#define SDK_BT_RF_CONFIG_SLEEP_MODE                  0xFD2B

/* Internal Functions */

/* API Declarations */

#ifdef VS_INIT_DEBUG

#define VS_INIT_TRC BT_debug_trace
#define VS_INIT_INF BT_debug_info
#define VS_INIT_ERR BT_debug_error

#else

#define VS_INIT_TRC BT_debug_null
#define VS_INIT_INF BT_debug_null
#define VS_INIT_ERR BT_debug_null

#endif /* VS_INIT_DEBUG */


#define LEVEL_0 0
#define LEVEL_1 1
#define LEVEL_2 2
#define LEVEL_3 3

#define CALIB_LEVEL_0 0
#define CALIB_LEVEL_1 1
#define CALIB_LEVEL_2 2

extern UINT32 vendor_specific_init_complete;

/**
 * @brief   vendor specific initialization function used from application
 * @param   void
 * @return  void
 * @see     vendor_specific_init.c
 */
void app_vendor_specific_init(void);
void set_power_level_params(UCHAR power_level, UCHAR * param);
void send_single_power_table(UCHAR output_power);

#endif /* _H_VENDOR_SPECIFIC_INIT_ */
