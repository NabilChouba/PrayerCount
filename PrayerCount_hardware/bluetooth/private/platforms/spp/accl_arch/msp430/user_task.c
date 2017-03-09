
/**
 * Copyright (C) 2009  MindTree Ltd.  All rights reserved.
 *
 * \file    user_task.c
 * \brief   This File contains the functions related to user task handling.
 */

/* Header File Inclusion */
#include "sdk_pl.h"
#include "appl_sdk.h"
#include "task.h"
#include "BT_task.h"




/* Extern variables */
extern volatile UINT32 inactivity_counter;
extern UCHAR debounce;
extern UCHAR sdk_bt_visible;
extern xSemaphoreHandle xUserSemaphore;
extern UCHAR sdk_initiator;
extern UCHAR sdk_connect_in_progress;
extern UCHAR LED_STATUS;

#ifdef MSP430_LPM_ENABLE
extern UCHAR lpm_mode;
#endif /* MSP430_LPM_ENABLE */



/* This structure is used for creating tasks related to BT application. To add
 * other tasks, use the direct xTaskCreate of FreeRTOS and then change BT task
 * priorities appropriately here Stack Size for the task is allocated from
 * heap_memory of heap_bt.c */

BT_OS_THREAD_PROPERTY bt_os_thread_property[] = {
    {USER_TASK_NAME, USER_TASK_PRIORITY, USER_TASK_STACK_SIZE}
    ,
    {WRITE_TASK_NAME, WRITE_TASK_PRIORITY, WRITE_TASK_STACK_SIZE}
    ,
    {READ_TASK_NAME, READ_TASK_PRIORITY, READ_TASK_STACK_SIZE}
};


/* Global variables */
UCHAR circular_user_buffer[MAX_USER_BUF];
INT16 circular_user_buf_rd = 0;
INT16 circular_user_buf_wt = 0;
volatile INT16 user_buf_len = 0;
static UCHAR inactivity_timeout = INACTIVITY_TIMEOUT;




/**
 *  \fn           user_task_routine
 *  \Description  Task to handle timer and menu related functionality
 *  \param        args
 *  \return       None
 */
void *user_task_routine(void *args)
{
    API_RESULT retval;

    UPDATE_USER_BUFFER(POWER_ON_RESET);
    user_buf_len = 1;
    BT_thread_mutex_unlock(&xUserSemaphore);

    while (1) {
        if (pdPASS == xSemaphoreTake(xUserSemaphore, 0xFFFF)) {
            while (user_buf_len > 0) {
                user_buf_len--;
                switch (circular_user_buffer[circular_user_buf_rd]) {
                case INACTIVITY_STATE:
                    inactivity_counter++;
                    break;
                case BT_DISCOVERABLE_STATE:
                    TOGGLE_LED2();
                    break;
                case SWITCH_S1:
                    /* 
                     * Switch SW1 is mapped to SPP connection/disconnection
                     * request. SW1 is a toggle switch.
                     */
                    sdk_bluetooth_menu_handler(OP_PEER_CONNECT);
                    break;
                case SWITCH_S2:
                    /* Switch SW2 is mapped to contol datasend stop and pause
                     * functionality */
                    sdk_bluetooth_menu_handler(OP_PEER_DATASEND);
                    break;
                case POWER_ON_RESET:
                    /* Turn ON the bluetooth controller */
                    BT_RF_NSHUTDOWN_PIN_HIGH();
                    /* Perform BT ON */
                    retval =
                        BT_bluetooth_on(sdk_hci_event_indication_callback,
                                        sdk_bluetooth_on_complete,
                                        (CHAR *) SDK_CONFIG_DEVICE_NAME);
                    if (retval != API_SUCCESS) {
                        sdk_display(SDK_MSG_AREA, (const UCHAR *)
                                    "Failed to turn on Bluetooth\n", 0);
                    }
                    break;
                default:
                    break;
                }
                circular_user_buf_rd++;
                if (circular_user_buf_rd == MAX_USER_BUF) {
                    circular_user_buf_rd = 0;
                }
            }

        }
    }
}


/**
 *  \fn           sdk_config_timer
 *  \Description  This function configures the timers used by the application
 *  \param        None
 *  \return       None
 */
void sdk_config_timer(void)
{
    /* Configure Timer1_A3 which is used for menu and Led indication related
     * functionalities */
    configTimer1_A3();
}

/**
 *  \fn             TIMER1_A0_ISR
*  \Description     This is Interrupt routine for Timer1_A3.This handles the menu and LED indications related functionalities
 *  \param          None
 *  \return         None
 */
#ifndef TOOLCHAIN_IAR
#pragma CODE_SECTION(TIMER1_A0_ISR,".ISR");
#endif /* TOOLCHAIN_IAR */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
    volatile INT16 bytes_rxed_in_user_buf = 0;
    signed portBASE_TYPE xHigherPriorityTaskWoken;

    UART_DISABLE_BT_UART_RTS();
    xHigherPriorityTaskWoken = pdFALSE;

    TA1CCTL0 &= ~CCIFG;
    __bis_SR_register(GIE);

    /* If the debounce flag is set, clear the flag to allow processing of next
     * button interrupts */
    if (1 == debounce) {
        debounce = 0;
    }

    /* Halting the Timer1_A3 */
    TA1CTL &= ~MC_3;

    if (TRUE == sdk_connect_in_progress) {
        /* Setting the inactivity timeout accordingly */
        inactivity_timeout = (INACTIVITY_TIMEOUT << 2);
        /* Reinitializing theTimer register value to generate interuupt every
         * 0.25 sec. This is used to indicate the user that the device is
         * trying to inquire and connect to BlueMSP demo device */
        TA1CCR0 = 0x2000;
    } else {
        /* Setting the inactivity timeout accordingly */
        inactivity_timeout = INACTIVITY_TIMEOUT;
        /* Reinitializing the Timer register value to generate interrupt every
         * 1 sec */
        TA1CCR0 = 0x8000;
    }

    /* Resume Timer1_A3 */
    TA1CTL |= MC_1;

    if (inactivity_counter < (inactivity_timeout + 1)) {
        UPDATE_USER_BUFFER(INACTIVITY_STATE);
    }

    if (SDK_IS_BT_DISCOVERABLE()) {
        UPDATE_USER_BUFFER(BT_DISCOVERABLE_STATE);
    }
    bytes_rxed_in_user_buf = circular_user_buf_wt - circular_user_buf_rd;
    /* To handle roll over condition of user buffer */
    if (bytes_rxed_in_user_buf < 0) {
        bytes_rxed_in_user_buf =
            (MAX_USER_BUF - circular_user_buf_rd) + circular_user_buf_wt;
    }

    /* Unlocking User Task */
    if (bytes_rxed_in_user_buf > 0) {
        user_buf_len = bytes_rxed_in_user_buf;
        /* Unblock the task by releasing the semaphore */
        xSemaphoreGiveFromISR(xUserSemaphore, &xHigherPriorityTaskWoken);
    }
#ifdef MSP430_LPM_ENABLE
    if (inactivity_timeout == inactivity_counter) {
        lpm_mode = TRUE;
        /* Save the LED pins status */
        LED_STATUS = LED_PORT_OUT & (LED_1 | LED_2);
        /* switch OFF the LEDs * */
        LED_PORT_OUT &= ~(LED_1 | LED_2);
        LED_PORT_DIR |= (LED_1 | LED_2);

        halAccStop();
        halI2CShutdown();
        
        /* Halting the Timer1_A3 */
        TA1CTL &= ~MC_3;
        /* Turn OFF MSP430 UART block only if ehcill is defined. UART will then
         * be enabled on ehcill wake up from the CTS port interrupt */
#ifdef SDK_EHCILL_MODE
        hci_uart_bt_shutdown();
#else
        /* If EHCILL is not defined RTS is enabled to handle exiting LPM from
         * peer device else CTS pulse is used to exit LPM */
        UART_ENABLE_BT_UART_RTS();
#endif /* SDK_EHCILL_MODE */

        /* Turn OFF UART1 which is used as USB serial port */
        halUsbShutDown();
        ENTER_MSP430_LPM();
    }
#endif /* MSP430_LPM_ENABLE */

    /* Force a context switch if xHigherPriorityTaskWoken was set to true */
    if (xHigherPriorityTaskWoken) {
        portYIELD();
    }
}
