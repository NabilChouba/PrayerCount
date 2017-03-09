
/**
 *  @file ehcill_pl.h
 *
 *  This file contains function/datatype specific to this ehcill mode
 */

/* 
 *  Copyright (C) 2009-2010. MindTree Ltd.
 *  All rights reserved.
 */

#ifndef _H_EHCILL_PL_
#define _H_EHCILL_PL_

/* ----------------------------------------------- Header File Inclusion */
#include "BT_hci_api.h"
#include "hci_uart.h"
#include "sdk_pl.h"

/* ----------------------------------------------- Macros */

/* BT controller vendor specific command to configure sleep mode */
#define SDK_BT_RF_SET_SLEEP_MODE                     0xFD0C
#define SDK_BT_RF_CONFIG_SLEEP_MODE                  0xFD2B

/* ehcill mode command opcodes */
#define SDK_BT_RF_SLEEP_IND         0x30
#define SDK_BT_RF_SLEEP_ACK         0x31
#define SDK_BT_RF_WAKE_UP_IND       0x32
#define SDK_BT_RF_WAKE_UP_ACK       0x33

/* ehcill mode states */
#define BT_RF_AWAKE_STATE               0x00
#define BT_RF_IN_DEEP_SLEEP             0x01
#define BT_RF_DEEP_SLEEP_IND_RCVD       0x02
#define BT_RF_WAKE_UP_IND_SENT          0x04
#define BT_RF_WAKE_UP_IND_RCVD          0x05

/* ----------------------------------------------- Functions */
/* Function to configure deep sleep mode */
API_RESULT sdk_deep_sleep_config(void);

/* Function to enable deep sleep mode */
API_RESULT sdk_deep_sleep_enable(void);

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

#endif /* _H_EHCILL_PL_ */
