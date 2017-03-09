/**
 * Copyright (C) 2010 Texas Instruments, Inc.
 *
 * \file    hal_buttons.h 
 * \brief   hal_buttons header file
 * \version July 2010
 */

#ifndef HAL_BUTTONS_H
#define HAL_BUTTONS_H

#define BUTTON_PORT_DIR   P2DIR
#define BUTTON_PORT_SEL   P2SEL
#define BUTTON_PORT_OUT   P2OUT
#define BUTTON_PORT_REN   P2REN
#define BUTTON_PORT_IE    P2IE
#define BUTTON_PORT_IES   P2IES
#define BUTTON_PORT_IFG   P2IFG
#define BUTTON_PORT_IN    P2IN

#define BUTTON_S1         BIT2 
#define BUTTON_S2         BIT3 
#define BUTTON_ALL        0x06

/*-------------------------------------------------------------
 *                  Function Prototypes 
 * ------------------------------------------------------------*/ 
void halButtonsInit(unsigned char buttonsMask);
unsigned char halButtonsPressed(void);
void halButtonsInterruptEnable(unsigned char buttonIntEnableMask);
void halButtonsInterruptDisable(unsigned char buttonIntEnableMask);
void halButtonsShutDown();

#endif
