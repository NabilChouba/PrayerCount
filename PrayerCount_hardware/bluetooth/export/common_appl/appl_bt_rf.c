
/**
 * Copyright (C) 2010. MindTree Ltd.  All rights reserved.
 *
 * \file    appl_bt_rf.c
 * \brief   This file contains functions related to ehcill and lps mode.
 */


/* Header File Inclusion */
#include "BT_common.h"
#include "sdk_config.h"
#include "appl_bt_rf.h"
#include "sdk_pl.h"
#include "vendor_specific_init.h"

extern API_RESULT BT_hci_vendor_specific_command(UINT16 ocf, UCHAR * params,
                                                 UCHAR params_length);
extern void MSP430_uart_init(void);
extern void hci_uart_write_data(UCHAR *, UINT16);

UCHAR deep_sleep_state = BT_RF_AWAKE_STATE;

/* LPS parameters */
static UCHAR sdk_lps_param[] = {
    /* Enable LPS */
    0x01,
    /* Number of scans required in order to get back into LPS */
    (UCHAR) SDK_LPS_DISABLE_SWEEPS_LENGTH,
    SDK_LPS_DISABLE_SWEEPS_LENGTH >> 8,
    /* The number of consequent LPS scans that trigger regular scan */
    SDK_LPS_POSITIVE_SWEEPS_TH,
    /* Enable LPS in active connection (Don't Change) */
    0x00,
    /* Minimum time between LPS scans in frames (1.25 milli secs) */
    SDK_LPS_MIN_SCAN_INTERVAL,
    /* Reserved */
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};

#ifdef SDK_EHCILL_MODE

/* Extern Function Declarations */
extern API_RESULT hci_trigger_command_tx(void);
extern void l2cap_schedule_data_tx(void);

/**
 * Enable Big sleep, Enable Deep Sleep, HCILL Mode, Do not change,
 * Do not change, Do not change, Do not change, Host_Wake deassertion timer
 */
static UCHAR sleep_enable_param[] =
    { 0x01, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x64, 0x00 };

/* Disable deep sleep param */
static UCHAR sleep_disable_param[] =
    { 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x64, 0x00 };

/**
 * inactivity_timeout (x * 1.25 milli secs),
 * retransmit_timeout (x * 1.25 milli secs),
 * rts_pulse_width (micro secs)
 */
static UCHAR sleep_config_param[] = {
    (UCHAR) SDK_EHCILL_INACTIVITY_TIMEOUT,
    (UCHAR) (SDK_EHCILL_INACTIVITY_TIMEOUT >> 8),
    (UCHAR) SDK_EHCILL_RETRANSMISSION_TIMEOUT,
    (UCHAR) (SDK_EHCILL_RETRANSMISSION_TIMEOUT >> 8),
    (UCHAR) SDK_EHCILL_RTS_PULSE_WIDTH,
    (UCHAR) (SDK_EHCILL_RTS_PULSE_WIDTH >> 8)
};

/**
 * \fn      uart_enable_cts_interrupt
 * \brief   Function used to set CTS in interript mode
 * \param   None
 * \return  void
 */
void uart_enable_cts_interrupt(void)
{
    /* Setting CTS Port Direction */
    *(bt_uart_config.uart_cts_port_dir) &= ~BT_UART_CTS_PIN;
    /* Enabling REN for CTS Pin */
    *(bt_uart_config.uart_cts_port_ren) |= BT_UART_CTS_PIN;
    /* Enabling PullDown Resister for CTS pin */
    *(bt_uart_config.uart_cts_port_out) &= ~BT_UART_CTS_PIN;

    /* Enabling Interrupt for high to low transistion */
    *(bt_uart_config.uart_cts_ies) |= BT_UART_CTS_PIN;
    /* Clearing Interrupt flag */
    *(bt_uart_config.uart_cts_ifg) &= ~BT_UART_CTS_PIN;
    /* Enabling Interrupt for CTS Pin */
    *(bt_uart_config.uart_cts_ie) |= BT_UART_CTS_PIN;
}

/**
 * \fn      uart_disable_cts_interrupt
 * \brief   Function used to set CTS pin to normal GPIO
 * \param   None
 * \return  void
 */
void uart_disable_cts_interrupt(void)
{
    /* Setting CTS Port Direction */
    *(bt_uart_config.uart_cts_port_dir) &= ~BT_UART_CTS_PIN;
    /* Enabling REN for CTS Pin */
    *(bt_uart_config.uart_cts_port_ren) |= BT_UART_CTS_PIN;
    /* Enabling PullDown Resistor for CTS pin */
    *(bt_uart_config.uart_cts_port_out) |= BT_UART_CTS_PIN;

    /* Clearing IFG Flag */
    *(bt_uart_config.uart_cts_ifg) &= ~BT_UART_CTS_PIN;
    /* Clearing Interrupt flag */
    *(bt_uart_config.uart_cts_ie) &= ~BT_UART_CTS_PIN;
}



/**
 * \fn      PORT1_VECTOR_ISR
 * \brief   The ISR is used for the ehcill wake up from CTS functionality
 *          According to Ehcill implementation , before going to sleep CTS pin of the host is configured as interrpt pin
 *          The contoller issues a pulse on the CTS signal to wake up the host.As CTS is acconected to PORT 1 pin , the pulse is detected as PORT 1 interrupt
 * \param   None
 * \return  void
 */
#ifndef TOOLCHAIN_IAR
#pragma CODE_SECTION(PORT1_VECTOR_ISR,".ISR");
#endif /* TOOLCHAIN_IAR */
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_VECTOR_ISR(void)
{
    /* Setup MSP430 UART before exiting LPM */
    MSP430_uart_init();
    /* Disable CTS interrupt */
    uart_disable_cts_interrupt();
    /* Enable MSP430 RTS */
    UART_ENABLE_BT_UART_RTS();
}


/**
 *  \fn     ehcill_send_wakeup_ind
 *  \brief  Function to send wake up indication to the controller
 *  \param  void
 *  \return void
 */
void ehcill_send_wakeup_ind(void)
{
    UCHAR cmd_opcode = SDK_BT_RF_WAKE_UP_IND;

    uart_disable_cts_interrupt();
    /* Check the deep sleep state */
    if (deep_sleep_state == BT_RF_IN_DEEP_SLEEP) {
        deep_sleep_state = BT_RF_WAKE_UP_IND_SENT;
        hci_uart_write_data(&cmd_opcode, 0x01);
        printf("Sending wake up ind to controller\n");
    } else {
        printf("Cannot send wake up ind to controller, invalid state %02X\n",
               deep_sleep_state);
    }
    UART_ENABLE_BT_UART_RTS();
}

/**
 *  \fn     ehcill_send_wakeup_ack
 *  \brief  Function to send wake up acknoledgment to the controller
 *  \param  void
 *  \return void
 */
void ehcill_send_wakeup_ack(void)
{

    if (deep_sleep_state == BT_RF_WAKE_UP_IND_RCVD) {
        printf("Sending wake up ack to controller\n");
        deep_sleep_state = BT_RF_AWAKE_STATE;
    } else {
        printf("Cannot send wake up ack to controller, invalid state %02X\n",
               deep_sleep_state);
    }
}


/**
 *  \fn     ehcill_send_deep_sleep_ack
 *  \brief  Function to send deep sleep acknowledgement to the controller
 *  \param  void
 *  \return void
 */
void ehcill_send_deep_sleep_ack(void)
{
    if (deep_sleep_state == BT_RF_DEEP_SLEEP_IND_RCVD) {
        printf("Sending deep sleep ack to controller\n");
        deep_sleep_state = BT_RF_IN_DEEP_SLEEP;
    } else {
        printf("Cannot send deep sleep ack to controller,invalid state %02X\n",
               deep_sleep_state);
    }
}


/**
 *  \fn     ehcill_handle_sleep_ind
 *  \brief  Function to handle deep sleep up indication from the controller
 *  \param  void
 *  \return void
 */
void ehcill_handle_sleep_ind(void)
{
    printf("Sleep ind received\n");

    if (deep_sleep_state == BT_RF_AWAKE_STATE) {
        deep_sleep_state = BT_RF_DEEP_SLEEP_IND_RCVD;
        /* Transfer deep sleep acknowledgement */
        ehcill_send_deep_sleep_ack();
    } else {
        printf("Ignoring \"Sleep ind\" received in wrong state %02X.\n",
               deep_sleep_state);
    }
}

/**
 *  \fn     ehcill_handle_wake_up_ind
 *  \brief  Function to handle wake up indication from the controller
 *  \param  void
 *  \return void
 */
void ehcill_handle_wake_up_ind(void)
{
    printf("wake up ind received\n");
    if (deep_sleep_state == BT_RF_IN_DEEP_SLEEP) {
        deep_sleep_state = BT_RF_WAKE_UP_IND_RCVD;
        /* Send the wake up acknowledgement */
        ehcill_send_wakeup_ack();
    } else if (deep_sleep_state == BT_RF_WAKE_UP_IND_SENT) {
        /* Treat this as wake up ack from controller */
        deep_sleep_state = BT_RF_AWAKE_STATE;
        /* Schedule command and data transfer */
        hci_trigger_command_tx();
        l2cap_schedule_data_tx();
    } else {
        printf("Ignoring \"Wake ind\" received in wrong state %02X.\n",
               deep_sleep_state);
    }
}

/**
 *  \fn     ehcill_handle_wake_up_ack
 *  \brief  Function to handle wake up acknowledgement from the controller
 *  \param  void
 *  \return void
 */
void ehcill_handle_wake_up_ack(void)
{
    printf("wake up ack received\n");
    if (deep_sleep_state == BT_RF_WAKE_UP_IND_SENT) {
        deep_sleep_state = BT_RF_AWAKE_STATE;
        /* Schedule command and data transfer */
        hci_trigger_command_tx();
        l2cap_schedule_data_tx();
    } else {
        printf("Ignoring \"Wake ind\" received in wrong state %02X.\n",
               deep_sleep_state);
    }

}

#endif /* SDK_EHCILL_MODE */

/**
 *  \fn     ehcill_uart_handler
 *  \brief  This function handles ehcill data on the host wake up
 *  \param  ehcill_data Data Received
 *  \return API_RESULT - API_SUCCESS or API_FAILURE
 */
API_RESULT ehcill_uart_handler(UCHAR ehcill_data)
{
    API_RESULT ret_val = API_FAILURE;
#ifdef SDK_EHCILL_MODE
    switch (ehcill_data) {
    case SDK_BT_RF_SLEEP_IND:
        uart_enable_cts_interrupt();
        ehcill_handle_sleep_ind();
        UART_DISABLE_BT_UART_TX();
        UART_TRANSMIT(SDK_BT_RF_SLEEP_ACK);
        ret_val = API_SUCCESS;
        break;
    case SDK_BT_RF_WAKE_UP_IND:
        /* HCILL_WAKE_UP_IND command received */
        ehcill_handle_wake_up_ind();
        UART_TRANSMIT(SDK_BT_RF_WAKE_UP_ACK);
        UART_ENABLE_BT_UART_RTS();
        ret_val = API_SUCCESS;
        break;
    case SDK_BT_RF_WAKE_UP_ACK:
        /* HCILL_WAKE_UP_ACK command received */
        ehcill_handle_wake_up_ack();
        UART_ENABLE_BT_UART_RTS();
        ret_val = API_SUCCESS;
        break;
    default:
        break;
    }
#endif
    return ret_val;
}

/**
 *  \fn     ehcill_wake_up_from_host
 *  \brief  This function handles ehcill data on the host wake up
 *  \param  ehcill_data Data Received
 *  \return API_RESULT - API_SUCCESS or API_FAILURE
 */

API_RESULT ehcill_wake_up_from_host(void)
{
#ifdef  SDK_EHCILL_MODE
    /* Check if ehcill state is sleep state */
    if (deep_sleep_state != BT_RF_AWAKE_STATE) {
        /* In deep sleep state, so wake up controller */
        ehcill_send_wakeup_ind();
        return API_SUCCESS;
    }
#endif
    return API_FAILURE;
}

/**
 * \fn      sdk_deep_sleep_config
 * \brief   Function to configure deep sleep mode
 * \param   None
 * \return  API_SUCCESS/API_FAILURE
 */
API_RESULT sdk_deep_sleep_config(void)
{
    API_RESULT retval = API_FAILURE;
#ifdef SDK_EHCILL_MODE

    retval =
        BT_hci_vendor_specific_command(SDK_BT_RF_CONFIG_SLEEP_MODE,
                                       sleep_config_param,
                                       sizeof(sleep_config_param)
        );
#endif
    return retval;
}


/**
 * \fn      sdk_deep_sleep_enable
 * \par     Function to enable deep sleep mode
 * \param   None
 * \return  API_SUCCESS/API_FAILURE
 */
API_RESULT sdk_deep_sleep_enable(void)
{
    API_RESULT retval = API_FAILURE;
#ifdef SDK_EHCILL_MODE
    retval =
        BT_hci_vendor_specific_command(SDK_BT_RF_SET_SLEEP_MODE,
                                       sleep_enable_param,
                                       sizeof(sleep_enable_param)
        );
#endif /* SDK_EHCILL_MODE */
    return retval;
}


/**
 * \fn      sdk_deep_sleep_disable
 * \brief   Function to disable deep sleep mode
 * \param   None
 * \return  API_SUCCESS/API_FAILURE
 */
API_RESULT sdk_deep_sleep_disable(void)
{
    API_RESULT retval = API_FAILURE;
#ifdef SDK_EHCILL_MODE

    retval =
        BT_hci_vendor_specific_command(SDK_BT_RF_SET_SLEEP_MODE,
                                       sleep_disable_param,
                                       sizeof(sleep_disable_param)
        );
#endif /* SDK_EHCILL_MODE */
    return retval;
}


/**
 * \fn      sdk_enable_lps_mode
 * \brief   Function which configures BT LPS mode.
 * \param   None
 * \return  API_SUCCESS/API_FAILURE
 */
API_RESULT sdk_enable_lps_mode()
{
    API_RESULT retval = API_FAILURE;
#ifdef SDK_LPS_MODE
    sdk_display(SDK_MSG_AREA,
                (const UCHAR *)"Enabling BT Low power Scan (LPS)...", 0);

    /* Configure LPS */
    retval =
        BT_hci_vendor_specific_command(SDK_BT_RF_SET_LPS_PARAMS, sdk_lps_param,
                                       sizeof(sdk_lps_param)
        );
#endif /* SDK_LPS_MODE */
    return retval;
}
