
/**
 * Copyright (C) 2009 MindTree Ltd.  All rights reserved.
 *
 * \file    appl_bt_rf.h
 * \brief   This file contains functions declarations related to ehcill protocol implementation
 */

#ifndef _H_APPL_BT_RF_
#define _H_APPL_BT_RF_

/* States pertaining to ehcill mode */
#define SDK_BT_RF_SLEEP_IND         0x30
#define SDK_BT_RF_SLEEP_ACK         0x31
#define SDK_BT_RF_WAKE_UP_IND       0x32
#define SDK_BT_RF_WAKE_UP_ACK       0x33

#define BT_RF_AWAKE_STATE               0x00
#define BT_RF_IN_DEEP_SLEEP             0x01
#define BT_RF_DEEP_SLEEP_IND_RCVD       0x02
#define BT_RF_WAKE_UP_IND_SENT          0x04
#define BT_RF_WAKE_UP_IND_RCVD          0x05

/* Function to configure deep sleep mode */
API_RESULT sdk_deep_sleep_config(void);

/* Function to enable deep sleep mode */
API_RESULT sdk_deep_sleep_enable(void);

#ifdef SDK_EHCILL_MODE

/* Function to disable deep sleep mode */
API_RESULT sdk_deep_sleep_disable(void);

/* Function to send wake up indication to the controller */
void ehcill_send_wakeup_ind(void);

/* Function to send wake up acknowledgment to the controller */
void ehcill_send_wakeup_ack(void);

/* Function to send deep sleep acknowledgement to the controller */
void ehcill_send_deep_sleep_ack(void);

/* Function to handle deep sleep up indication from the controller */
void ehcill_handle_sleep_ind(void);

/* Function to handle wake up indication from the controller */
void ehcill_handle_wake_up_ind(void);

/* Function to handle wake up acknowledgement from the controller */
void ehcill_handle_wake_up_ack(void);
#else

#define ehcill_send_wakeup_ind()
#define ehcill_send_wakeup_ack()
#define ehcill_send_deep_sleep_ack()
#define ehcill_handle_sleep_ind()
#define ehcill_handle_wake_up_ind()
#define ehcill_handle_wake_up_ack()

#endif /* SDK_EHCILL_MODE */

/* Function to handle ehcill data in the read task */
API_RESULT ehcill_uart_handler(UCHAR ehcill_data);

/* Funtion to check echill state to wake up the controller */
API_RESULT ehcill_wake_up_from_host(void);

#endif /* _H_APPL_BT_RF_ */
