
/**
 * Copyright (C) 2009 MindTree Ltd.  All rights reserved.
 *
 * \file    appl_menu_pl.c
 * \brief   This file contains button interrupt and related handling
 */

#include "appl_sdk.h"
#include "task.h"


/* Global Variables */
UCHAR debounce = 0;


/* Extern Variables */
#ifdef TOOLCHAIN_IAR
extern void __program_start();
#else /* CCSV4 */
extern void __interrupt _c_int00();
#endif /* TOOLCHAIN_IAR */

extern volatile UINT32 inactivity_counter;
extern UCHAR circular_user_buffer[];
extern INT16 circular_user_buf_wt;
extern INT16 circular_user_buf_rd;
extern volatile INT16 user_buf_len;

extern xSemaphoreHandle xUserSemaphore;

/* Global Variables */

/** Local Bluetooth Device Name */
CHAR sdk_local_name[SDK_REM_BT_DEV_NAME_MAX_LEN] = "NAME";
UCHAR state = 1;
UINT8 switch_pos = 0;
UCHAR lpm_mode = FALSE;

/* Functions Declarations */

/* Functions */

/**
 * \brief 		initializes the MSP430 board (S1 and S2) button related registers
 *
 * \param[in]	void
 *
 * \return		none	
 **/
void init_buttons(void)
{
    debounce = 1;

    BUTTON_PORT_DIR &= ~(BUTTON_S1 + BUTTON_S2);
    BUTTON_PORT_REN |= (BUTTON_S1 + BUTTON_S2);
    BUTTON_PORT_OUT |= BUTTON_S1 + BUTTON_S2;
    BUTTON_PORT_SEL &= ~(BUTTON_S1 + BUTTON_S2);
    BUTTON_PORT_IES &= ~(BUTTON_S1 + BUTTON_S2);
    BUTTON_PORT_IFG &= ~(BUTTON_S1 + BUTTON_S2);
    BUTTON_PORT_IE |= (BUTTON_S1 + BUTTON_S2);
}

/**
 * \brief   ISR routine for handling PORT2 interrupts. It handles switch
 *          debounce and switching modes for LPM
 *
 * \return		none	
 **/
#ifndef TOOLCHAIN_IAR
#pragma CODE_SECTION(PORT2_VECTOR_ISR,".ISR");
#endif /* TOOLCHAIN_IAR */
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_VECTOR_ISR(void)
{
    volatile INT16 bytes_rxed_in_user_buf = 0;
    signed portBASE_TYPE xHigherPriorityTaskWoken;

    UART_DISABLE_BT_UART_RTS();

    xHigherPriorityTaskWoken = pdFALSE;

    switch_pos = P2IV;
    __bis_SR_register(GIE);

    inactivity_counter = 0;

    if (debounce == 0) {
        /* to take care of switch debounce */
        debounce = 1;

        if (FALSE == lpm_mode) {
            UPDATE_USER_BUFFER(switch_pos);
            bytes_rxed_in_user_buf =
                circular_user_buf_wt - circular_user_buf_rd;
            /* To handle roll over condition of user buffer */
            if (bytes_rxed_in_user_buf < 0) {
                bytes_rxed_in_user_buf =
                    (MAX_USER_BUF - circular_user_buf_rd) +
                    circular_user_buf_wt;
            }

            /* Unlocking User Task */
            if (bytes_rxed_in_user_buf > 0) {
                user_buf_len = bytes_rxed_in_user_buf;
                /* Unblock the user task by releasing the semaphore */
                xSemaphoreGiveFromISR(xUserSemaphore,
                                      &xHigherPriorityTaskWoken);
            }
        }

    }


    /* If the processor is in LPM, move to active mode */
#ifdef MSP430_LPM_ENABLE
    if (TRUE == lpm_mode) {
        lpm_mode = FALSE;
        restore_peripheral_status();
        EXIT_MSP430_LPM();
    }
#endif /* MSP430_LPM_ENABLE */

    /* Force a context switch if xHigherPriorityTaskWoken was set to true */
    if (xHigherPriorityTaskWoken) {
        portYIELD();
    }
}
