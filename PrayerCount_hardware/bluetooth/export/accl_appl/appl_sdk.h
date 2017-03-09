/* 
 *  Copyright (C) 2009-2010. MindTree Ltd.
 *  All rights reserved.
 */

/**
 *
 *  This header file contains function/datatype declaration for Default
 *  Accelerometer Application
 */


#ifndef _H_APPL_SDK_
#define _H_APPL_SDK_

/* ----------------------------------------------- Header File Inclusion */
#include "sdk_pl.h"
#include "BT_api.h"
#include "appl_spp.h"

#define MAX_USER_BUF    20

/* ----------------------------------------- Macros */
#define appl_hci_print_bd_addr(bd)      appl_dump_bytes((bd), BT_BD_ADDR_SIZE);

/* Connection States */
#define SDK_IN_ACL_CONNECTION         0x01
#define SDK_ACL_CONNECTED             0x02
#define SDK_IN_SDP_QUERY              0x03
#define SDK_IN_SPP_CONNECTION         0x04
#define SDK_SPP_CONNECTED             0x05
#define SDK_IN_SPP_DISCONNECTION      0x06
#define SDK_IN_ACL_DISCONNECTION      0x07
#define SDK_SPP_DISCONNECTED          0x08
#define SDK_DISCONNECTED              0x09

/* Macro to check spp connection status */
#define SDK_IS_SPP_CONNECTED(index)   \
    (SDK_SPP_CONNECTED == sdk_status[(index)].connect_switch)

/* Macro to check acl connection status */
#define SDK_IS_DISCONNECTED(index)   \
    (SDK_DISCONNECTED == sdk_status[(index)].connect_switch)

/* Macro to check spp connection status */
#define SDK_IS_SPP_DISCONNECTED(index)   \
    (SDK_SPP_DISCONNECTED == sdk_status[(index)].connect_switch)

/* Macro to check spp connection status */
#define SDK_IS_SPP_IN_DISCONNECTION(index)   \
    (SDK_IN_SPP_DISCONNECTION == sdk_status[(index)].connect_switch)

/* Macro to check spp connection status */
#define SDK_IS_IN_ACL_CONNECTION(index)   \
    (SDK_IN_ACL_CONNECTION == sdk_status[(index)].connect_switch)

/* Macro to check spp link status */
#define SDK_IS_IN_ACTIVE_MODE(index)   \
    (SDK_ACTIVE == sdk_status[(index)].link_state)

#ifdef SDK_ENABLE_SNIFF_MODE
/* Macro to check spp link status */
#define SDK_IS_IN_SNIFF_MODE(index)   \
    (SDK_IN_SNIFF == sdk_status[(index)].link_state)
#else
#define SDK_IS_IN_SNIFF_MODE(index) FALSE
#endif /* SDK_ENABLE_SNIFF_MODE */

/* Macro to check data to be sent flag state */
#define SDK_IS_IN_DATA_SEND_STATE(index) \
    (TRUE == sdk_status[(index)].appl_spp_data_to_be_sent)

/* Change SPP Connection Status */
#define SDK_SPP_CHANGE_STATE(index, state)   \
    (sdk_status[(index)].connect_switch = state)
/* Change SPP Data transfer state */
#define SDK_SPP_CHANGE_TX_STATE(index, state)   \
    sdk_status[(index)].sdk_data_sending = state;
/* Change SPP Link state */
#define SDK_SPP_CHANGE_LINK_STATE(index, state)   \
    sdk_status[(index)].link_state = state;
/* Change data to be sent flag state */
#define SDK_SPP_CHANGE_DATA_STATE(index, state) \
    sdk_status[(index)].appl_spp_data_to_be_sent = state;

/* ----------------------------------------- Structures/Data Types */
/* Structure to store remote bluetooth devices info found during inquiry */
typedef struct {
    /* Remote Device Bluetooth address */
    UCHAR bd_addr[BT_BD_ADDR_SIZE];

    /* Remote Device Clock offset */
    UINT16 clock_offset;

    /* Remote Device page scan mode */
    UCHAR page_scan_rep_mode;

} SDK_REM_BD_DEVICES;

/* Structure which stores the status of the SPP Connections if any */
typedef struct {
    /* SPP Connection handle */
    UINT16 spp_connection_handle;

    /* ACL Connection handle */
    UINT16 acl_connection_handle;

    /* Bluetooth Device Address of peer */
    UCHAR peer_bd_addr[BT_BD_ADDR_SIZE];

    /* Bluetooth Connection Status Flag */
    UCHAR connect_switch;

    /* Bluetooth Data Send Status Flag */
    UCHAR sdk_data_sending;
#ifdef SDK_ENABLE_SNIFF_MODE
    /* SDK ACL Link status */
    UCHAR link_state;
#endif                          /* SDK_ENABLE_SNIFF_MODE */
    /* Flag to check if data transfer was halted because of l2cap buffer
     * overflow */
    UCHAR appl_spp_data_to_be_sent;

} SDK_SPP_CONNECTION_STATUS;
/* ----------------------------------------------- Functions */
/* Initialize the sdk status flags */
void sdk_appl_init(void);
API_RESULT sdk_bluetooth_on_complete(void);

API_RESULT sdk_hci_event_indication_callback(UCHAR event_type,
                                             UCHAR * event_data,
                                             UCHAR event_datalen);

API_RESULT sdk_sm_ui_notify_cb(UCHAR event_type, UCHAR * bd_addr,
                               UCHAR * event_data);

void appl_dump_bytes(UCHAR * buffer, UINT16 length);
void appl_bluetooth_on_complete_event_handler(void);

void sdk_bluetooth_menu_handler(UCHAR input);

void appl_acl_connection_complete_event(UCHAR * bd_addr, UCHAR status,
                                        UINT16 connection_handle);

/* Function to read from accelerometer and send over SPP */
void appl_acceleromenter_read_spp_send(UCHAR rem_bt_dev_index);

/* Function to get free instance in sdk_status array */
API_RESULT appl_get_free_status_instance(UCHAR * id);

/* Function to get spp connection instance based on acl connection handle */
API_RESULT appl_get_status_instance_acl(UCHAR * id,
                                        UINT16 acl_connection_handle);

/* Function to get spp connection instance based on SPP connection handle */
API_RESULT appl_get_status_instance_spp(UCHAR * id,
                                        UINT16 spp_connection_handle);

/* Function to get spp connection instance based on peer bd adddress */
API_RESULT appl_get_status_instance_bd_addr(UCHAR * id, UCHAR * rem_bd_addr);

#endif /* _H_APPL_ACCELEROMETER_ */
