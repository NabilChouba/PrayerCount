/**
 * Copyright (C) 2010 Texas Instruments, Inc.
 *
 * \file    hal_usb.h 
 * \brief   hal_usb.c header file
 * \version July 2010
 */

#ifndef HAL_USB_H
#define HAL_USB_H


#define USB_PORT_OUT      P10OUT
#define USB_PORT_SEL      P10SEL
#define USB_PORT_DIR      P10DIR
#define USB_PORT_REN      P10REN
#define USB_PIN_TXD       BIT4
#define USB_PIN_RXD       BIT5

/*-------------------------------------------------------------
 *                  Function Prototypes
 * ------------------------------------------------------------*/
void halUsbInit(unsigned char clock);
void halUsbShutDown(void);
void halUsbSendChar(unsigned char character);
void halUsbSendString(unsigned char string[], unsigned char length);

#endif
