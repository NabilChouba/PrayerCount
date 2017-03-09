/**
 * Copyright (C) 2010 Texas Instruments, Inc.
 *
 * \file    hal_usb.c
 * \brief   This file contains functions to handle UART-USB connection
 * \version July 2010
 */

#include  <msp430x54xA.h>
#include "hal_EZ430-RF2560.h"
#include "portmacro.h"


/* Global variables */
char halUsbReceiveBuffer[255];
unsigned char bufferSize = 0;

/* Function definitions */

/**
 * \fn      halUsbInit
 * \brief   Initializes the serial communications peripheral and GPIO ports
 *          to communicate with the TUSB3410.
 * \param   none
 * \return  none
 */
void halUsbInit(unsigned char clock)
{
    volatile unsigned char i;

    for (i = 0; i < 255; i++)
        halUsbReceiveBuffer[i] = '\0';

    bufferSize = 0;
    USB_PORT_SEL |= USB_PIN_RXD + USB_PIN_TXD;
    USB_PORT_DIR |= USB_PIN_TXD;
    USB_PORT_DIR &= ~USB_PIN_RXD;

    UCA3CTL1 |= UCSWRST;        // Reset State 
    UCA3CTL0 = UCMODE_0;        /* UART mode */

    UCA3CTL0 &= ~UC7BIT;        // 8bit char
    UCA3CTL1 |= UCSSEL_2;
    switch (clock) {  // Set BR Low byte on system clock
      case (SYSCLK_1MHZ):
          UCA3BRW = 0x68;   // (9600 Baud)  
          UCA3MCTL = 0x01;  // 0x00; // 0x01;
          break;
  
      case (SYSCLK_4MHZ):
          UCA3BRW = 0x01A0; // (9600 Baud) 
          UCA3MCTL = 0x06;  // 0x00; // 0x06;
          break;
  
      case (SYSCLK_8MHZ):
          UCA3BRW = 0x0341; // (9600 Baud) 
          UCA3MCTL = 0x02;  // 0x00; // 0x02;
          break;  
  
      case (SYSCLK_12MHZ):
          UCA3BR0 = 226;    // (9600 Baud)
          UCA3BR1 = 4;
          UCA3MCTL = 0x00;  // 0x00; // 0x00;
          break;     
    
      case (SYSCLK_18MHZ):
          UCA3BRW = 0x0753; // (9600 Baud)
          UCA3MCTL = 0x00;  // 0x00; // 0x00;
          break;
  
      case (SYSCLK_20MHZ):
          UCA3BRW = 0x0823; // (9600 Baud)
          UCA3MCTL = 0x06;  // 0x00; // 0x06;
          break;
  
      case (SYSCLK_25MHZ):
          UCA3BRW = 0x0A2C; // (9600 Baud)
          UCA3MCTL = 0x02;  // 0x00; // 0x02;
          break;
    }
    UCA3CTL1 &= ~UCSWRST;
    UCA3IE |= UCRXIE;

}                                                                                                                /**

 * \fn      halUsbShutDown
 * \brief   Disables the serial communications peripheral and clears the GPIO
 *          settings used to communicate with the TUSB3410.
 * \param   none
 * \return  none
 */
void halUsbShutDown(void)
{
    UCA3IE &= ~UCRXIE;
    UCA3CTL1 = UCSWRST;         // Reset State 
    USB_PORT_SEL &= ~(USB_PIN_RXD + USB_PIN_TXD);
    USB_PORT_DIR |= USB_PIN_TXD;
    USB_PORT_DIR |= USB_PIN_RXD;
    USB_PORT_OUT &= ~(USB_PIN_TXD + USB_PIN_RXD);
}

                                                                                                                 /**
 * \fn      halUsbSendChar
 * \brief   Sends a character over UART to the TUSB3410.
 * \param   character The character to be sent.
 * \return  none
 */
void halUsbSendChar(unsigned char character)
{
    while (!(UCA3IFG & UCTXIFG));
    UCA3TXBUF = character;
}

                                                                                                                /**
 * \fn      halUsbSendString
 * \brief   Sends a string of characters to the TUSB3410
 * \param   string[] The array of characters to be transmit to the TUSB3410.
 * \param   length   The length of the string.
 * \return  none
 */
void halUsbSendString(unsigned char string[], unsigned char length)
{
    volatile unsigned char i;
    for (i = 0; i < length; i++) {
        halUsbSendChar(string[i]);
    }
}

/**
 * \fn      USCI_A1_VECTOR
 * \brief   This is the interrupt routine for USCI A1
 *          In the routine the byte received on the UART A1 is copied to a receive buffer
 * \param   none
 * \return  none
 */
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    halUsbReceiveBuffer[bufferSize++] = UCA1RXBUF;
    __bic_SR_register_on_exit(LPM3_bits);
}


