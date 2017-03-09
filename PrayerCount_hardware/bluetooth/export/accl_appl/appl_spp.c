
/**
 * Copyright (c) 2009 MindTree Ltd.  All rights reserved.
 * \file    appl_spp.c
 * \brief   This file contains SPP specific function definitions
 */

/* Header File Inclusion */
#include "appl_spp.h"
#include "l2cap.h"
#include "appl_sdk.h"
#include "task.h"
#include "appl_bt_rf.h"

/* Extern variables */
extern UCHAR deep_sleep_state;
extern UCHAR circular_user_buffer[];
extern INT16 circular_user_buf_wt;
extern xSemaphoreHandle xUserSemaphore;
extern volatile INT16 user_buf_len;

/* spp connections status information */
extern SDK_SPP_CONNECTION_STATUS sdk_status[];
/* Flag to check if USB is connected */
extern UCHAR sdk_usb_detected;
/* Flag to check if the device is SPP Initiator */
extern UCHAR sdk_initiator;
/* Flag to indicate that SPP connection is in progress */
extern UCHAR sdk_connect_in_progress;

/* Static Global Variables */
/* User Context (HANDLE) used for SDP query to find out SPP record */
static SDP_HANDLE appl_spp_sdp_handle;
static UCHAR appl_spp_remote_server_ch;
static UCHAR appl_spp_local_server_ch;
static volatile INT16 bytes_rxed_in_read_buf;
static volatile INT16 bytes_rxed_in_user_buf = 0;
/* Based on the L2CAP Tx Buffer Flow this state will be set */
static UCHAR appl_l2cap_tx_buf_state = L2CAP_TX_QUEUE_FLOW_ON;

/* Functions */

/**
 * @brief   Function to initiate SDP query after a successful ACL connection
 * @param   rem_bt_dev_index Index of the remote BT device
 * @return  void
 * @see     appl_spp.c
 */
void appl_spp_sdp_query(UCHAR rem_bt_dev_index)
{
    API_RESULT retval;

    /* 
     *  SDP Query Should be done only after the Previously Initiated
     *  ACL Connection Completes Successfully.
     *
     *  Start SDP Query to Find out if the Peer Device is running SPP
     *  Service and if so on which Server Channel the Remote SPP Server
     *  is accepting connection.
     *
     *  In this Non-Blocking Implemenation, the whole process of
     *  Performing SDP Query is boot-straped by first establishing
     *  an SDP Connection with the Peer.
     *
     *  At the time of Requesting SDP to Establish a Connection, the
     *  Application also Registers a Callback function with SDP, using
     *  that Callback function SDP Module will keep indicating
     *  Asynchronous SDP events to the Application.
     *
     *  The Application Callback for SDP is the Third argument of
     *  the SDP_SET_HANDLE() Macro.
     */

    /* Set the SDP Handle MAKE IT GLOBAL */
    SDP_SET_HANDLE(appl_spp_sdp_handle,
                   sdk_status[rem_bt_dev_index].peer_bd_addr,
                   appl_spp_sdp_callback);

    /* Set the Peer Server Channel with invalid value */
    appl_spp_remote_server_ch = 0xFF;

    /* Open SDP Connection */
    printf("Initiating SDP Connection ... ");

    retval = BT_sdp_open(&appl_spp_sdp_handle);

    if (retval != API_SUCCESS) {
        printf("** FAILED ** !! Reason Code = 0x%04X\n", retval);

        SDK_SPP_CHANGE_STATE(rem_bt_dev_index, SDK_ACL_CONNECTED);
        printf("Initiating ACL disconnection\n");
        /* Disconnect ACL */
        retval =
            BT_hci_disconnect(sdk_status
                              [rem_bt_dev_index].acl_connection_handle, 0x13);
        if (retval != API_SUCCESS) {
            printf("** FAILED ** !! Reason Code = 0x%04X\n", retval);
        }
    } else {
        printf("OK.\n");
    }

}

/**
 * @brief   callback function for sdp command
 * @param   command SDP command
 * @param   data Buffer with paramters
 * @param   length Length of data buffers
 * @param   status Status of the SDP command
 * @return  void
 * @see     appl_spp.c
 */
void appl_spp_sdp_callback(UINT8 command, UINT8 * data, UINT16 length,
                           UINT16 status)
{
    S_UUID uuid[1];
    UINT16 num_uuids;
    UINT16 attrib_id[1];
    UINT16 num_attribute_ids;
    API_RESULT retval;

    /** Buffer to hold SPP SDP Attribute Data */
    UCHAR *appl_spp_attrib_data;
    UINT16 appl_spp_attrib_data_len;

    /* Handle failures */
    if (status != API_SUCCESS) {
        printf("*** SDP Command 0x%02X: 0x%04X\n", command, status);

        if (command == SDP_ServiceSearchAttributeResponse) {
            /* Close SDP Connection */
            BT_sdp_close(&appl_spp_sdp_handle);
        }

        return;
    }

    switch (command) {
    case SDP_Open:
        /* Initialize UUID for SPP */
        uuid[0].uuid_type = UUID_16;
        uuid[0].uuid_union.uuid_16 = SERIALPORT_UUID;
        num_uuids = 0x01;

        /* Initiate Attribute ID for SPP */
        attrib_id[0] = PROTOCOL_DESC_LIST;
        num_attribute_ids = 0x01;

        /* Allocate Memory */
        appl_spp_attrib_data_len = SDK_SPP_ATTRIB_DATA_LEN;
        appl_spp_attrib_data = BT_alloc_mem(appl_spp_attrib_data_len);

        if (NULL == appl_spp_attrib_data) {
            printf("SPP Open Failed, reason %04X\n", status);
        } else {

            /* Initiate SDP Service Search Attribute Request for SPP */
            printf("Initiating SDP SSA Request for SPP ... ");

            retval =
                BT_sdp_servicesearchattributerequest(&appl_spp_sdp_handle, uuid,
                                                     num_uuids, attrib_id,
                                                     num_attribute_ids, NULL,
                                                     0x00, appl_spp_attrib_data,
                                                     &appl_spp_attrib_data_len);

            if (retval != API_SUCCESS) {
                printf("** FAILED ** !! Reason Code = 0x%04X\n", retval);

                /* Free Memory */
                BT_free_mem(appl_spp_attrib_data);

                /* Close SDP Connection */
                BT_sdp_close(&appl_spp_sdp_handle);
            } else {
                printf("OK.\n");
            }
        }
        break;

    case SDP_ServiceSearchAttributeResponse:
        /* Get/Decode RFCOMM Server Channel value */
        printf("Received Response for SDP SSA Request for SPP.\n");
        printf("Getting SPP Server Channel ... ");

        retval = BT_sdp_get_channel_number(data, &appl_spp_remote_server_ch);

        if (retval != API_SUCCESS) {
            printf("*** FAILED to Get Server Channel Value: 0x%04X\n", retval);
        } else {
            printf("OK.\n");
            printf("SPP Server Channel: 0x%02X\n", appl_spp_remote_server_ch);

            /* 
             * Mark flag to Initiate SPP Connection
             * after SDP disconnection.
             *
             * SPP Connection can be initiated from here too,
             * but it will done after SDP disconnection, for
             * better interoperability.
             *
             * Extracted Server Channel itself can be used as flag
             * to save memory
             */
        }

        /* Free Memory */
        BT_free_mem(data);

        /* Close SDP Connection */
        BT_sdp_close(&appl_spp_sdp_handle);

        break;

    case SDP_Close:            /* Fall Through */
        /* Check if the SPP Server Channel is extracted */
        if (0xFF != appl_spp_remote_server_ch) {
            /* Initiate SPP Connection */
            appl_spp_connect(appl_spp_sdp_handle.bd_addr,
                             appl_spp_remote_server_ch);
        }
        break;

    default:
        break;
    }

}

/**
 * @brief   Function to initiate SPP connection with a peer
 * @param   bd_addr BD address of peer
 * @param   server_ch Server channel
 * @return  void
 * @see     appl_spp.c
 */
void appl_spp_connect(UCHAR * bd_addr, UCHAR server_ch)
{
    API_RESULT retval;
    UCHAR rem_bt_dev_index;

    /* Get the remote device status instance */
    retval = appl_get_status_instance_bd_addr(&rem_bt_dev_index, bd_addr);

    if (retval != API_SUCCESS) {
        printf("Unknown remote device\n");
    } else {

        printf("Initiating SPP Connection ... ");
        retval = BT_spp_connect(bd_addr, server_ch);

        if (retval != API_SUCCESS) {
            printf("** FAILED ** !! Reason Code = 0x%04X\n", retval);
            printf("Initiating ACL disconnection\n");
            /* Disconnect ACL */
            retval =
                BT_hci_disconnect(sdk_status
                                  [rem_bt_dev_index].acl_connection_handle,
                                  0x13);
            if (retval != API_SUCCESS) {
                printf("** FAILED ** !! Reason Code = 0x%04X\n", retval);
            }
        } else {
            printf("OK.\n");
            SDK_SPP_CHANGE_STATE(rem_bt_dev_index, SDK_IN_SPP_CONNECTION);
        }
    }
}

/*************************************************************************
* @brief Function to initate a SPP disconnetion with a peer
*
* @param  rem_bt_dev_index Index of peer device
*
* @return void
*
* @see   appl_spp.c
*************************************************************************/
void appl_spp_disconnect(UCHAR rem_bt_dev_index)
{
    API_RESULT retval;

    /* Check if there exist SPP Connection */
    /* Do SPP Close */
    printf("Initiating SPP Disconnections ... ");
    retval =
        BT_spp_disconnect(sdk_status[rem_bt_dev_index].spp_connection_handle);

    if (retval != API_SUCCESS) {
        printf("** FAILED ** !! Reason Code = 0x%04X\n", retval);
        printf("Initiating ACL disconnection\n");
        /* Disconnect ACL */
        retval =
            BT_hci_disconnect(sdk_status
                              [rem_bt_dev_index].acl_connection_handle, 0x13);
        if (retval != API_SUCCESS) {
            printf("** FAILED ** !! Reason Code = 0x%04X\n", retval);
        }
    } else {
        printf("OK.\n");
        SDK_SPP_CHANGE_STATE(rem_bt_dev_index, SDK_IN_SPP_DISCONNECTION);
    }
}

/**
 * @brief Function to handle SPP indication
 * @param  handle Handle of SPP connection
 * @param  event_type Type of SPP event
 * @param  result Result
 * @param  data Data buffer to decode
 * @param  datalen Length of data buffer
 * @return API_SUCCESS/API_FAILURE
 * @see   appl_spp.c
 */
API_RESULT appl_spp_notify_cb(IN SPP_HANDLE handle, IN SPP_EVENTS event_type,
                              IN API_RESULT result, IN void *data,
                              IN UINT16 datalen)
{
    UCHAR *l_data;
    API_RESULT retval;
    UINT8 index;
    UCHAR rem_bt_dev_index;

    /* String to store the SPP dat received */
    UCHAR string[4];
    l_data = (UCHAR *) (data);


    printf("\n" "SPP HANDLE : %u\n" "EVENT      : %d\n" "RESULT     : 0x%04X\n",
           (unsigned int)handle, event_type, result);
    if ((event_type == SPP_CONNECT_CNF) || (event_type == SPP_CONNECT_IND)) {
        /* Get the remote device status instance based on rem bd addr */
        appl_get_status_instance_bd_addr(&rem_bt_dev_index, l_data);
    } else {
        /* Get the remote device status instance based on spp connection handle 
         */
        retval = appl_get_status_instance_spp(&rem_bt_dev_index, handle);
        if (retval != API_SUCCESS) {
            /* Unknown SPP connection instance */
            return retval;
        }
    }

    if (API_SUCCESS != result) {
        printf("\nSPP Failure\n");

        if (SPP_CONNECT_CNF == event_type) {
            /* Try Reconnect SPP */
            appl_spp_sdp_query(rem_bt_dev_index);
        }
        return API_FAILURE;
    }

    switch (event_type) {
    case SPP_CONNECT_CNF:
        sdk_initiator = TRUE;
        appl_spp_connect_indication();
        printf("SPP_CONNECT_CNF -> Connection Successful\n");
        printf("Remote device BD_ADDR : %02X:%02X:%02X:%02X:%02X:%02X\n",
               l_data[0], l_data[1], l_data[2], l_data[3], l_data[4],
               l_data[5]);

        if (API_SUCCESS == result) {
            /* Save SPP Handle and Change State to SPP Connected */
            sdk_status[rem_bt_dev_index].spp_connection_handle = handle;
            SDK_SPP_CHANGE_STATE(rem_bt_dev_index, SDK_SPP_CONNECTED);
            /* SPP Connection complete so reset the flag */
            sdk_connect_in_progress = FALSE;

#ifdef SDK_ENABLE_SNIFF_MODE
            retval =
                BT_hci_sniff_mode(sdk_status
                                  [rem_bt_dev_index].acl_connection_handle,
                                  SDK_CONFIG_SNIFF_MAX_INTERVAL,
                                  SDK_CONFIG_SNIFF_MIN_INTERVAL,
                                  SDK_CONFIG_SNIFF_ATTEMPT,
                                  SDK_CONFIG_SNIFF_TIMEOUT);
            if (retval == API_SUCCESS) {
                printf("Started sniff mode\n");
                break;
            }
            printf("Failed to start sniff mode\n");
#endif /* SDK_ENABLE_SNIFF_MODE */
        }
        break;

    case SPP_CONNECT_IND:
        sdk_initiator = FALSE;
        appl_spp_connect_indication();
        printf("SPP_CONNECT_IND -> Connection Successful\n");
        printf("Remote device BD_ADDR : %02X:%02X:%02X:%02X:%02X:%02X\n",
               l_data[0], l_data[1], l_data[2], l_data[3], l_data[4],
               l_data[5]);

        if (API_SUCCESS == result) {
            /* Save SPP Handle and Change State to SPP Connected */
            sdk_status[rem_bt_dev_index].spp_connection_handle = handle;
            SDK_SPP_CHANGE_STATE(rem_bt_dev_index, SDK_SPP_CONNECTED);
#ifdef SDK_ENABLE_SNIFF_MODE
            retval =
                BT_hci_sniff_mode(sdk_status
                                  [rem_bt_dev_index].acl_connection_handle,
                                  SDK_CONFIG_SNIFF_MAX_INTERVAL,
                                  SDK_CONFIG_SNIFF_MIN_INTERVAL,
                                  SDK_CONFIG_SNIFF_ATTEMPT,
                                  SDK_CONFIG_SNIFF_TIMEOUT);
            if (retval == API_SUCCESS) {
                printf("Started sniff mode\n");
                break;
            }
            printf("Failed to start sniff mode\n");
#endif /* SDK_ENABLE_SNIFF_MODE */
            /* To reflect the changes in the SPP cionnection to the user */
        }
        /* TBD: Reset state on failure */

        break;

    case SPP_DISCONNECT_CNF:
        printf("SPP_DISCONNECT_CNF -> Disconnection Successful\n");
        printf("Remote device BD_ADDR : %02X:%02X:%02X:%02X:%02X:%02X\n",
               l_data[0], l_data[1], l_data[2], l_data[3], l_data[4],
               l_data[5]);

        SDK_SPP_CHANGE_STATE(rem_bt_dev_index, SDK_SPP_DISCONNECTED);

        SDK_SPP_CHANGE_TX_STATE(rem_bt_dev_index, SDK_SPP_TX_OFF);
#ifdef SDK_ENABLE_SNIFF_MODE
        SDK_SPP_CHANGE_LINK_STATE(rem_bt_dev_index, SDK_OFF);
#endif /* SDK_ENABLE_SNIFF_MODE */
        /* Initiate acl disconnection */
        printf("Initiating ACL disconnection\n");
        retval =
            BT_hci_disconnect(sdk_status[rem_bt_dev_index].
                              acl_connection_handle, 0x13);
        if (retval != API_SUCCESS) {
            printf("** FAILED ** !! Reason Code = 0x%04X\n", retval);
        }
        break;

    case SPP_DISCONNECT_IND:
        printf("SPP_DISCONNECT_IND -> Disconnection Successful\n");
        printf("Remote device BD_ADDR : %02X:%02X:%02X:%02X:%02X:%02X\n",
               l_data[0], l_data[1], l_data[2], l_data[3], l_data[4],
               l_data[5]);

        SDK_SPP_CHANGE_STATE(rem_bt_dev_index, SDK_SPP_DISCONNECTED);
        /* To reflect the changes in the SPP cionnection to the user */
        SDK_SPP_CHANGE_TX_STATE(rem_bt_dev_index, SDK_SPP_TX_OFF);
#ifdef SDK_ENABLE_SNIFF_MODE
        SDK_SPP_CHANGE_LINK_STATE(rem_bt_dev_index, SDK_OFF);
#endif /* SDK_ENABLE_SNIFF_MODE */
        break;

    case SPP_STOP_CNF:
        printf("SPP_STOP_CNF -> Stop Successful\n");
        break;

    case SPP_SEND_CNF:
        printf("SPP_SEND_CNF -> Sent successfully\n");

        if (SDK_IS_SPP_CONNECTED(rem_bt_dev_index)
            && SDK_IS_SPP_TX_STARTED(rem_bt_dev_index)) {
            if (L2CAP_TX_QUEUE_FLOW_ON == appl_l2cap_tx_buf_state) {
                appl_acceleromenter_read_spp_send(rem_bt_dev_index);
            } else {
                SDK_SPP_CHANGE_DATA_STATE(rem_bt_dev_index, TRUE);
            }
        }
        break;

    case SPP_RECVD_DATA_IND:
        printf("SPP_RECVD_DATA_IND -> Data received successfully\n");
        printf("\n----------------HEX DUMP------------------------\n");
        for (index = 0; index < datalen; index++) {
            printf("%02X ", l_data[index]);
        }
        printf("\n------------------------------------------------\n");


        if (TRUE == sdk_usb_detected) {
            memcpy(string, l_data, datalen);
            //halUsbSendString(string, 4);
            //halUsbSendString("1234", 4);
        }
        UCHAR appl_accelerometer_buffer[6];
        appl_spp_write(rem_bt_dev_index, "OK ",3);
        //sdk_accelerometer_read(appl_accelerometer_buffer,sizeof(appl_accelerometer_buffer));
        //halUsbSendString(appl_accelerometer_buffer,sizeof(appl_accelerometer_buffer));
        //halUsbSendString("\n",1);
        //appl_spp_write(rem_bt_dev_index,appl_accelerometer_buffer,sizeof(appl_accelerometer_buffer));

        break;

    default:
        printf("\nUnknown command type\n");
        break;
    }                           /* switch */

    return API_SUCCESS;
}

/**
 * @brief Function to write data on a SPP connection
 * @param  rem_bt_dev_index Index of peer BT device
 * @param  data Pointer to data buffer
 * @param  data_len Length of the data buffer
 * @return API_SUCCESS/API_FAILURE
 * @see   appl_spp.c
 */
API_RESULT appl_spp_write(UCHAR rem_bt_dev_index, UCHAR * data, UINT16 data_len)
{
    return BT_spp_send(sdk_status[rem_bt_dev_index].spp_connection_handle, data,
                       data_len);
}

/**
 * @brief   Function to init and start SPP from application
 * @param   none
 * @return  void
 * @see     appl_spp.c
 */
void appl_spp_start(void)
{
    API_RESULT retval;
    UINT32 appl_spp_record_handle;
    SM_SERVICE appl_spp_sm_service;
    UCHAR appl_spp_sm_service_id;

    /* Get record handle */
    retval =
        BT_dbase_get_record_handle(DB_RECORD_SPP, 0, &appl_spp_record_handle);

    if (retval != API_SUCCESS) {
        printf("*** FAILED to Get SPP Record Handle: 0x%04X\n", retval);
        return;
    }

    /* Get Server Channel from SPP Service Record */
    retval =
        BT_dbase_get_server_channel(appl_spp_record_handle, PROTOCOL_DESC_LIST,
                                    &appl_spp_local_server_ch);

    if (retval != API_SUCCESS) {
        printf("*** FAILED to Get SPP Server Channel: 0x%04X\n", retval);
    } else {

        /**
         * Add RFCOMM Service (with SPP Server Channel) security needs to the
         * SM Service Database
         */
        appl_spp_sm_service.psm = RFCOMM_PSM;
        appl_spp_sm_service.server_ch = appl_spp_local_server_ch;
        /* Authentication level */
        appl_spp_sm_service.authenticate = SDK_CONFIG_SPP_AUTHENTICATION_LEVEL;
        /* Authorization Not Required */
        appl_spp_sm_service.authorize = SDK_CONFIG_SPP_AUTHORIZATION_REQ;
        /* Encryption is always ON under SSP */
        appl_spp_sm_service.encrypt = 0x1;
        /* Must Register UI Callback for SSP */
        appl_spp_sm_service.service_sm_cb = appl_sm_service_cb;
        /* No Bonding */
        appl_spp_sm_service.no_bonding = SDK_CONFIG_SPP_BONDING_REQ;

        retval =
            BT_sm_add_service(&appl_spp_sm_service, &appl_spp_sm_service_id);
        if (retval != API_SUCCESS) {
            printf("*** FAILED to add RFCOMM Service security needs: 0x%04X\n",
                   retval);
        }

        /* Start SPP */
        retval = BT_spp_start(appl_spp_local_server_ch);

        if (API_SUCCESS != retval) {
            printf("Failed to Start SPP, reason %04X\n", retval);
        }

        /* Activate SPP Service Record - TODO */
        retval = BT_dbase_activate_record(appl_spp_record_handle);

        if (retval != API_SUCCESS) {
            printf("*** FAILED to Activate SPP Service Record: 0x%04X\n",
                   retval);
        } else {

            l2cap_register_tx_queue_flow_cb(appl_spp_l2cap_tx_queue_flow_cb);
        }
    }
}

/**
 * @brief   Function to stop SPP from application
 * @param   none
 * @return  void
 * @see     appl_spp.c
 */
void appl_spp_stop(void)
{
    BT_spp_stop();
}

/**
 * @brief   Callback funtion for L2cap TX queue flow for SPP data
 * @param   none
 * @return  void
 * @see     appl_spp.c
 */
API_RESULT appl_spp_l2cap_tx_queue_flow_cb(UCHAR state, UINT16 count)
{
    UCHAR index;

    /* Set the local tx state */
    appl_l2cap_tx_buf_state = state;

    /* Flow On - Check if data to be sent */
    if (L2CAP_TX_QUEUE_FLOW_ON == state) {
        for (index = 0; index < SPP_MAX_ENTITY; index++) {
            /* Check if data to be sent */
            if (SDK_IS_IN_DATA_SEND_STATE(index)) {
                if (SDK_IS_SPP_CONNECTED(index) && SDK_IS_SPP_TX_STARTED(index)) {
                    appl_acceleromenter_read_spp_send(index);
                }

                SDK_SPP_CHANGE_DATA_STATE(index, FALSE);
            }
        }
    }
    /* Flow Off - Don't send data */
    else {
        /* Set data can't be sent */
    }

    return API_SUCCESS;
}

/**
 * \fn      appl_sm_service_cb
 * \brief   Callback function for registered RFCOMM service with SM.
 * \param   event_type Type of event
 * \param   bd_addr Address of peer device
 * \param   event_data Data buffer for events
 * \return  API_SUCCESS/API_FAILURE
 * \see     appl_spp.c
 */
API_RESULT appl_sm_service_cb(UCHAR event_type, UCHAR * bd_addr,
                              UCHAR * event_data)
{
    API_RESULT retval;
    UCHAR appl_sm_ui_authorization_request_reply;

    printf("\n");
    printf("Received SM Service UI Notification. Event Type 0x%02X\n",
           event_type);

    retval = API_SUCCESS;

    switch (event_type) {
    case SM_AUTHORIZATION_REQUEST_NTF:
        printf("Received UI Authorization Request from SM\n");
        printf("BD_ADDR = %02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0],
               bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);

        /* Accept the authorization request */
        appl_sm_ui_authorization_request_reply = 0x01;

        printf("Replying to UI Authorization Request ... ");

        retval =
            BT_sm_authorization_request_reply(bd_addr,
                                              appl_sm_ui_authorization_request_reply);

        if (retval != API_SUCCESS) {
            printf("FAILED ! Reason = 0x%04X\n", retval);
            break;
        } else {
            printf("OK\n");
        }

        break;
    default:
        printf("*** Unknown/Undefined Event Type 0x%02X\n", event_type);
        break;
    }

    return retval;
}

/**
 * @brief Idle hook function for application
 * @param  none
 * @return void
 * @see   appl_spp.c
 */
void vApplicationIdleHook(void)
{
    while (1) {
#ifdef SDK_EHCILL_MODE
        if (BT_RF_IN_DEEP_SLEEP != deep_sleep_state) {
            UART_ENABLE_BT_UART_RTS();
        }
#else
        UART_ENABLE_BT_UART_RTS();
#endif /* SDK_EHCILL_MODE */
    }
}
