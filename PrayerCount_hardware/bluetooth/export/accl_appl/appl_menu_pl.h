/* 
 *  Copyright (C) 2009-2010. MindTree Ltd.
 *  All rights reserved.
 */

/**
 *  @file appl_menu_pl.h
 *
 *  @brief This file contains function/datatype specific to menu handling in MSP430
 */



#ifndef _H_APPL_MENU_PL_
#define _H_APPL_MENU_PL_

/* ----------------------------------------------- Header File Inclusion */
#include "hal_EZ430-RF2560.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
/* ----------------------------------------------- Macros */

/* 
 * button interrupt vector (P2Iv) values for
 * differnt switches
 */
#define SWITCH_S1         0x06
#define SWITCH_S2         0x08
#define POWER_ON_RESET    0x11

#define INACTIVITY_STATE        0x20
#define BT_DISCOVERABLE_STATE   0x21

#define MOVE_CURSOR_UP		0x00
#define MOVE_CURSOR_DOWN	0x01

/***************************************************/
/* Define the MAIN_MENU elements */
#define OP_INVALID 				  0xFF
#define OP_PEER_CONNECT 				0x01
#define OP_PEER_DATASEND                0x02
typedef void (*FP) (void);

/* Enumeration for output power level */
typedef enum {

    /**
     * Currently only 6dBm and 10dBm power levels are supported. 
     * The commented power levels will be added later after inputs from TI
     * OP_POWER_4_0,
     * OP_POWER_4_5,
     * OP_POWER_5_0,
     * OP_POWER_5_5,
     * OP_POWER_6_5,
     * OP_POWER_7_0,
     * OP_POWER_7_5,
     * OP_POWER_8_0,
     * OP_POWER_8_5,
     * OP_POWER_9_0,
     * OP_POWER_9_5,
     * OP_POWER_10_5,
     * OP_POWER_11_0,
     * OP_POWER_11_5,
     * OP_POWER_12_0
    */
    OP_POWER_6_0,
    OP_POWER_10_0
} OUTPUT_POWER_LEVEL;

/* ----------------------------------------------- Functions */
/* initialize LCD on MSP430 board */
void init_buttons(void);
#endif /* _H_APPL_MENU_PL_ */
