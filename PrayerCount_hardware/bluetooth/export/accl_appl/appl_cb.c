
/**
 * Copyright (c) 2009 MindTree Ltd.  All rights reserved.
 * \file    appl_bt_on_cb.c
 * \brief This file contains BT_bluetooth_on completion callback handler
 */


/* Header File Inclusion */
#include "appl_sdk.h"
#include "appl_bt_rf.h"
#include "sdk_config.h"

/* Extern Variables */
/* Bluetooth Power On/Off Status Flag */
extern UCHAR sdk_bt_power;
/* Falg to indicate application has requested for LPS configuration */
extern UCHAR sdk_config_lps_flag;
/* Extern Functions */
extern API_RESULT sdk_enable_lps_mode();

/* Extern Variables */
/* Bluetooth Discoverable On/Off Status Flag */
extern UCHAR sdk_bt_visible;

extern UCHAR deep_sleep_state;
/* spp connections status information */
extern SDK_SPP_CONNECTION_STATUS sdk_status[SPP_MAX_ENTITY];
extern UART_CONFIG_PARAMS bt_uart_config;
extern UCHAR sdk_initiator;
/* Flag to indicated that ACL connection is initiated by peer */
extern UCHAR peer_initiated_acl_connection;

/* Global Variables */

/* Used to Store Inquiry Results */
SDK_REM_BD_DEVICES rem_bt_dev[SDK_INQ_MAX_DEVICES];
UCHAR rem_dev_index = 0, rem_dev_num = 0;
/* Falg to indicate application has requested for LPS configuration */
UCHAR sdk_config_lps_flag = FALSE;

/* Flag to check if scan enable command is sent as part of BT ON */
UCHAR sdk_scan_enable = 0x00;


/**
 * @brief   Callback function on completion of Bluetooth being switched on
 * @param   none
 * @return  API_RESULT; API_SUCCESS on success, API_FAILURE otherwise
 * @see     appl_bt_on_cb.c
 */
API_RESULT sdk_bluetooth_on_complete(void)
{

    API_RESULT retval;

#ifdef BT_INIT_TIME_CALC
    BT_INIT_TIME_CALC_END();
#endif /* BT_INIT_TIME_CALC */


    /* Bluetooth ON Completed */
    sdk_bt_power = SDK_BT_ON;
    appl_bluetooth_on_indication();

    sdk_display(SDK_MSG_AREA,
                (const UCHAR *)"Bluetooth ON Initialization Completed.\n", 0);

    /* Register SM callback handler */
    sdk_display(SDK_MSG_AREA,
                (const UCHAR *)"Registering UI Notification Callback ... ", 0);
    retval = BT_sm_register_user_interface(sdk_sm_ui_notify_cb);

    /* 
     * All SM Events will be notified to the application
     * using sdk_sm_ui_notify_cb()
     */
    if (retval != API_SUCCESS) {
        sdk_display(SDK_MSG_AREA, (const UCHAR *)"Registration with SM FAILED",
                    0);
    }

    /* Set the Device IO Capability */
    retval = BT_sm_set_local_io_cap(SDK_IO_CAPABILITY);

    if (retval != API_SUCCESS) {
        sdk_display(SDK_MSG_AREA, (const UCHAR *)"Set IO Capability failed\0",
                    0);
    }

    /* If ehcill not enabled then enable LPS straight away */
    if (API_FAILURE == sdk_enable_lps_mode()) {
        /* Set the default link policy */
        BT_hci_write_default_link_policy_settings
            (SDK_CONFIG_LINK_POLICY_SETTINGS);
        appl_bluetooth_on_complete_event_handler();
    }



    return API_SUCCESS;
}


/**
* \brief    Callback function to decode and handle HCI event indications
* \param    event_type      Type of event indication
* \param    event_data      Pointer to buffer containing event byte stream
* \param    event_datalen Length of the event buffer
* \return   API_RESULT; API_SUCCESS on success, API_FAILURE otherwise
* \see      appl_hci_cb.c
*/
API_RESULT sdk_hci_event_indication_callback(UCHAR event_type,
                                             UCHAR * event_data,
                                             UCHAR event_datalen)
{
    UINT16 connection_handle, value_2;
    UCHAR status, value_1, link_type, num_responses, count, index,
        *rem_dev_name;
    UCHAR *bd_addr, dev_index, rem_dev_name_len, flag = 0;
    /* User selected remote bt device index */
    UCHAR rem_bt_dev_index;
    API_RESULT retval;
    /* Populate Inquiry scan access codes list(iac) with user defined value */
    UINT32 iac[] = { SDK_INQUIRY_SCAN_LAP };

    /* Switch on the Event Code */
    switch (event_type) {
    case HCI_CONNECTION_COMPLETE_EVENT:
        sdk_display(SDK_MSG_AREA,
                    (const UCHAR *)"Received HCI_CONNECTION_COMPLETE_EVENT.\n",
                    0);

        /* Status */
        hci_unpack_1_byte_param(&status, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tStatus: 0x%02X\n", status, 0); */
        event_data += 1;

        printf("Status = %02x\n", status);
        /* Connection Handle */
        hci_unpack_2_byte_param(&connection_handle, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tConnection Handle: 0x%04X\n",
         * connection_handle); */
        event_data += 2;

        /* BD_ADDR */
        sdk_display(SDK_MSG_AREA, (const UCHAR *)"\tBD_ADDR: ", 0);
        appl_hci_print_bd_addr(event_data);
        bd_addr = event_data;
        event_data += 6;

        /* Link Type */
        hci_unpack_1_byte_param(&link_type, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tLink Type: 0x%02X", link_type); */
        switch (link_type) {
        case HCI_SCO_LINK:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> SCO\n", 0);
            break;
        case HCI_ACL_LINK:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> ACL\n", 0);
            break;
#ifdef BT_HCI_1_2
        case HCI_ESCO_LINK:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> eSCO\n", 0);
            break;
#endif /* BT_HCI_1_2 */
        default:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> ???\n", 0);
            break;
        }
        event_data += 1;

        /* Encryption Mode */
        hci_unpack_1_byte_param(&value_1, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tEcnryption Mode: 0x%02X", value_1); */
        switch (value_1) {
        case 0x00:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> Encryption OFF\n", 0);
            break;
        case 0x01:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> Encryption ON\n", 0);
            break;
        default:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> ???\n", 0);
            break;
        }
        event_data += 1;

        if (HCI_ACL_LINK == link_type) {
            if (status != 0x00) {
                /* Try Reconnect ACL */
                BT_hci_create_connection(bd_addr, SDK_CONFIG_ACL_PKT_TYPE,
                                         rem_bt_dev[rem_dev_index].
                                         page_scan_rep_mode, 0,
                                         rem_bt_dev[rem_dev_index].clock_offset,
                                         0x01);
                break;
            }

            /* Make local device invisible */
            retval = BT_hci_write_scan_enable(0x00);

            if (API_SUCCESS != retval) {
                sdk_display(SDK_MSG_AREA,
                            (const UCHAR *)"Failed to turn off visibility\n",
                            0);
            } else {
                sdk_bt_visible = SDK_DISC_OFF;
            }

            /* Set the link super vision timeout */
            retval =
                BT_hci_write_link_supervision_timeout(connection_handle,
                                                      SDK_CONFIG_LINK_SUPERVISION_TIMEOUT);

            appl_acl_connection_complete_event(bd_addr, status,
                                               connection_handle);
        }
        break;

    case HCI_DISCONNECTION_COMPLETE_EVENT:
        sdk_display(SDK_MSG_AREA, (const UCHAR *)
                    "Received HCI_DISCONNECTION_COMPLETE_EVENT.\n", 0);

        /* Status */
        hci_unpack_1_byte_param(&status, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tStatus: 0x%02X\n", status); */
        event_data += 1;

        /* Connection Handle */
        hci_unpack_2_byte_param(&connection_handle, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tConnection Handle: 0x%04X\n",
         * connection_handle); */
        event_data += 2;

        /* Reason */
        hci_unpack_1_byte_param(&value_1, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tReason: 0x%02X\n", value_1); */
        event_data += 1;

        if (status == 0x00) {
            appl_get_status_instance_acl(&dev_index, connection_handle);
            SDK_SPP_CHANGE_STATE(dev_index, SDK_DISCONNECTED);

            retval = BT_hci_write_scan_enable(0x03);

            if (API_SUCCESS == retval) {
                sdk_bt_visible = SDK_DISC_ON;

                /**
                 * SPP reconnection not initiated if it is user (Local or
                 * Remote) terminated connection.
                 */
                if ((sdk_initiator) && ((value_1 != 0x16) && (value_1 != 0x13))) {
                    sdk_bluetooth_menu_handler(OP_PEER_CONNECT);
                }
                break;
            }
        }
        break;

    case HCI_COMMAND_COMPLETE_EVENT:
        sdk_display(SDK_MSG_AREA,
                    (const UCHAR *)"Received HCI_COMMAND_COMPLETE_EVENT.\n", 0);

        /* Number of Command Packets */
        hci_unpack_1_byte_param(&value_1, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tNum Command Packets: %d (0x%02X)\n",
         * value_1, value_1); */
        event_data += 1;

        /* Command Opcode */
        hci_unpack_2_byte_param(&value_2, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tCommand Opcode: 0x%04X\n", */
        /* value_2); */
        event_data += 2;

        /* Command Status */
        hci_unpack_1_byte_param(&status, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tCommand Status: 0x%02X\n", status); */
        event_data += 1;
        if (HCI_WRITE_INQUIRY_SCAN_TYPE_OPCODE == value_2) {
            /* Write Page Scan mode (Interlaced scanning) */
            retval = BT_hci_write_page_scan_type(0x01);
            if (API_SUCCESS != retval) {
                sdk_display(SDK_MSG_AREA, (const UCHAR *)
                            "Failed to turn set inquiry scan type\n", 0);
            }
        }

        if (HCI_WRITE_PAGE_SCAN_TYPE_OPCODE == value_2) {
            /* Write Inquiry Scan access codes */
            retval =
                BT_hci_write_current_iac_lap((sizeof(iac) / sizeof(UINT32)),
                                             iac);
            if (API_SUCCESS != retval) {
                sdk_display(SDK_MSG_AREA, (const UCHAR *)
                            "Failed to write inquiry access codes\n", 0);
            }
        }
        if (HCI_WRITE_CURRENT_IAC_LAP_OPCODE == value_2) {

            /* Set Class of Device */
            retval = BT_hci_write_class_of_device(SDK_CONFIG_COD);
            if (retval != API_SUCCESS) {
                sdk_display(SDK_MSG_AREA, (const UCHAR *)
                            "Failed to set class of device (COD)\n", 0);
            }

            /* Make Local Device Visible */
            retval = BT_hci_write_scan_enable(0x03);

            if (API_SUCCESS != retval) {
                sdk_display(SDK_MSG_AREA,
                            (const UCHAR *)"Failed to turn on visibility\n", 0);
            } else {

                /**
                 * LED: Inquiry and page scan mode enabled, Indicated by
                 * flashing of LED2 (Red)
                 */
                sdk_bt_visible = SDK_DISC_ON;
                sdk_scan_enable = 0x01;
            }
        }
        if ((value_2 == HCI_WRITE_SCAN_ENABLE_OPCODE)
            && (sdk_scan_enable == 0x01)) {
            retval = sdk_set_max_output_power(SDK_MAX_OUTPUT_POWER_LEVEL);
            if (retval != API_SUCCESS) {
                sdk_display(SDK_MSG_AREA, (const UCHAR *)
                            "Failed to set max power level\n", 0);
            }
            sdk_scan_enable = 0x00;
        }


        if (SDK_BT_RF_SET_LPS_PARAMS == value_2) {
            /* Set the default link policy */
            BT_hci_write_default_link_policy_settings
                (SDK_CONFIG_LINK_POLICY_SETTINGS);
            appl_bluetooth_on_complete_event_handler();
        }
        break;

    case HCI_COMMAND_STATUS_EVENT:
        sdk_display(SDK_MSG_AREA,
                    (const UCHAR *)"Received HCI_COMMAND_STATUS_EVENT.\n", 0);

        /* Status */
        hci_unpack_1_byte_param(&status, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tCommand Status: 0x%02X\n", status); */
        event_data += 1;

        /* Number of Command Packets */
        hci_unpack_1_byte_param(&value_1, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tNum Command Packets: %d (0x%02X)\n",
         * value_1, value_1); */
        event_data += 1;

        /* Command Opcode */
        hci_unpack_2_byte_param(&value_2, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tCommand Opcode: 0x%04X \n", */
        /* value_2); */
        event_data += 2;

#ifdef SDK_ENABLE_SNIFF_MODE
        if (HCI_SNIFF_MODE_OPCODE == value_2) {
            if (status == 0x00) {
                sdk_display(SDK_MSG_AREA,
                            (const UCHAR *)"Sniff mode enabled successfully.\n",
                            0);
            } else {
                sdk_display(SDK_MSG_AREA,
                            (const UCHAR *)"Enabling Sniff mode failed.\n", 0);
            }
            break;
        }
        if (HCI_EXIT_SNIFF_MODE_OPCODE == value_2) {
            if (status == 0x00) {
                sdk_display(SDK_MSG_AREA,
                            (const UCHAR *)"Exiting Sniff mode successfully.\n",
                            0);
            } else {
                sdk_display(SDK_MSG_AREA,
                            (const UCHAR *)"Exiting Sniff mode failed.\n", 0);
            }
            break;
        }
#endif /* SDK_ENABLE_SNIFF_MODE */

        /**
         * If remote name request during the inquiry fails for a device,
         * issue remote name request for the next discovered device if any
         */
        if ((API_SUCCESS != status)
            && (HCI_REMOTE_NAME_REQUEST_OPCODE == value_2)) {
            rem_dev_index++;
            if (rem_dev_index < rem_dev_num) {
                /* Device exist for which name request has not been issued */
                retval =
                    BT_hci_remote_name_request(rem_bt_dev[rem_dev_index].
                                               bd_addr,
                                               rem_bt_dev
                                               [rem_dev_index].page_scan_rep_mode,
                                               0x00,
                                               rem_bt_dev
                                               [rem_dev_index].clock_offset);

                if (retval != API_SUCCESS) {
                    printf
                        ("Remote Name Request FAILED !! Error Code = 0x%04x\n",
                         retval);
                }
            } else if (rem_dev_index == rem_dev_num) {
                /* Restart Inquiry as BlueMSP430Demo device is not found */
                retval =
                    BT_hci_inquiry(SDK_INQUIRY_LAP, SDK_INQUIRY_LEN,
                                   SDK_NUM_RESPONSES);
                if (retval != API_SUCCESS) {
                    sdk_display(SDK_MSG_AREA, (const UCHAR *)
                                "Failed to initiate Inquiry\n", 0);
                } else {
                    rem_dev_index = rem_dev_num = 0;
                    sdk_display(SDK_MSG_AREA, (const UCHAR *)
                                "Inquiry Started...Wait for Completion\n", 0);
                }
            }
        }
        break;
    case HCI_ROLE_CHANGE_EVENT:
        sdk_display(SDK_MSG_AREA, (const UCHAR *)
                    "Received HCI_ROLE_CHANGE_EVENT.\n", 0);
        /* Status */
        hci_unpack_1_byte_param(&status, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tStatus: 0x%02X\n", status); */
        event_data += 1;
        /* BD_ADDR of Remote Device */
        sdk_display(SDK_MSG_AREA, (const UCHAR *)"\tBD_ADDR: ", 0);
        appl_hci_print_bd_addr(event_data);
        event_data += 6;
        /* New Role */
        hci_unpack_1_byte_param(&value_1, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tNew Role: 0x%02X", value_1); */
        switch (value_1) {
        case 0x00:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> Master\n", 0);
            break;
        case 0x01:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> Slave\n", 0);
            break;
        default:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> ???\n", 0);
            break;
        }
        event_data += 1;
        break;
    case HCI_MODE_CHANGE_EVENT:
        sdk_display(SDK_MSG_AREA, (const UCHAR *)
                    "Received HCI_MODE_CHANGE_EVENT.\n", 0);
        /* Status */
        hci_unpack_1_byte_param(&status, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tStatus: 0x%02X\n", status); */
        event_data += 1;
        /* Connection Handle */
        hci_unpack_2_byte_param(&connection_handle, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tConnection Handle: 0x%04X\n",
         * connection_handle); */
        event_data += 2;
        /* Current Mode */
        hci_unpack_1_byte_param(&value_1, event_data);
        retval = appl_get_status_instance_acl(&dev_index, connection_handle);
        if (retval != API_SUCCESS) {
            printf("Spurious ACL Connection handle %04X!!!\n",
                   connection_handle);
            break;
        }
        /* sdk_display(SDK_STATUS_AREA,"\tCurrent Mode: 0x%02X", value_1); */
        switch (value_1) {
        case 0x00:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> Active Mode\n", 0);
#ifdef SDK_ENABLE_SNIFF_MODE
            SDK_SPP_CHANGE_LINK_STATE(dev_index, SDK_ACTIVE);
            if (SDK_IS_SPP_IN_DISCONNECTION(dev_index)) {
                /* Initiate Disconnection */
                appl_spp_disconnect(dev_index);
            }
#endif /* SDK_ENABLE_SNIFF_MODE */
            break;
        case 0x01:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> Hold Mode\n", 0);
            break;
        case 0x02:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> Sniff Mode\n", 0);
#ifdef SDK_ENABLE_SNIFF_MODE
            SDK_SPP_CHANGE_LINK_STATE(dev_index, SDK_IN_SNIFF);
            if (SDK_IS_SPP_TX_STARTED(dev_index)) {
                /* Start Sending Data */
                appl_acceleromenter_read_spp_send(dev_index);
            }
#endif /* SDK_ENABLE_SNIFF_MODE */
            break;
        case 0x03:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> Park Mode\n", 0);
            break;
        default:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> ???\n", 0);
            break;
        }
        event_data += 1;
        /* Interval */
        hci_unpack_2_byte_param(&value_2, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tInterval: 0x%04X\n", value_2); */
        event_data += 2;
        break;
    case HCI_LINK_KEY_NOTIFICATION_EVENT:
        sdk_display(SDK_MSG_AREA, (const UCHAR *)
                    "Received HCI_LINK_KEY_NOTIFICATION_EVENT.\n", 0);
        /* BD_ADDR */
        sdk_display(SDK_MSG_AREA, (const UCHAR *)"\tBD_ADDR: ", 0);
        appl_hci_print_bd_addr(event_data);
        event_data += 6;
        /* Link Key */
        sdk_display(SDK_MSG_AREA, (const UCHAR *)"\tLink Key: ", 0);
        appl_dump_bytes(event_data, 16);
        event_data += 16;
        /* Key Type */
        hci_unpack_1_byte_param(&value_1, event_data);
        /* sdk_display(SDK_STATUS_AREA,"\tKey Type: 0x%02X", value_1); */
        switch (value_1) {
        case 0x00:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> Combination Key\n",
                        0);
            break;
        case 0x01:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> Local Unit Key\n", 0);
            break;
        case 0x02:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> Remote Unit Key\n",
                        0);
            break;
        default:
            sdk_display(SDK_MSG_AREA, (const UCHAR *)" -> ???\n", 0);
            break;
        }
        event_data += 1;
        break;
    case HCI_INQUIRY_COMPLETE_EVENT:
        /* Status */
        hci_unpack_1_byte_param(&status, event_data);
        /* printf("Received HCI_INQUIRY_COMPLETE_EVENT.\n"); */
        /* printf("\tStatus: 0x%02X\n", status); */
        /* Inquiry Completed */
        if (0 == rem_dev_num) {
            /* No remote BT device found, so restart inquiry procedure */
            retval =
                BT_hci_inquiry(SDK_INQUIRY_LAP, SDK_INQUIRY_LEN,
                               SDK_NUM_RESPONSES);
            if (retval != API_SUCCESS) {
                sdk_display(SDK_MSG_AREA,
                            (const UCHAR *)"Failed to initiate Inquiry\n", 0);
            } else {
                rem_dev_index = rem_dev_num = 0;
                sdk_display(SDK_MSG_AREA, (const UCHAR *)
                            "Inquiry Started...Wait for Completion\n", 0);
            }
        } else {
            /* BT devices found, initiate remote name request for the first
             * device */
            retval =
                BT_hci_remote_name_request(rem_bt_dev[0].bd_addr,
                                           rem_bt_dev[0].page_scan_rep_mode,
                                           0x00, rem_bt_dev[0].clock_offset);
            if (retval != API_SUCCESS) {
                printf("Remote Name Request FAILED !! Error Code = 0x%04x\n",
                       retval);
            } else {
                printf("Remote BT Devices List:\n");
            }
        }
        break;
    case HCI_INQUIRY_RESULT_EVENT:

        /* printf("Received HCI_INQUIRY_RESULT_EVENT.\n"); */

        /* Number of Responses */
        hci_unpack_1_byte_param(&num_responses, event_data);
        /* printf("\tNum Response: %d (0x%02X)\n", num_responses,
         * num_responses); */
        event_data += 1;
        /* For each Response, Print the Inquiry Result */
        for (count = 0; count < num_responses; count++) {
            /* BD_ADDR of the Remote Device */
            /* printf("\tBD_ADDR: "); */
            /* appl_hci_print_bd_addr(event_data); */
            /* Note the bd_addr */
            bd_addr = event_data;
            /* printf ("%02X %02X %02X %02X %02X %02X\n", bd_addr[0],
             * bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]); */
            event_data += 6;
            /* Page Scan Repetition Mode */
            hci_unpack_1_byte_param(&value_1, event_data);
            /* printf("\tPage Scan Repetition Mode: 0x%02X\n", value_1); */
            event_data += 6;
            /* Clock Offset */
            hci_unpack_2_byte_param(&value_2, event_data);
            /* printf("\tClock Offset: 0x%04X\n", value_2); */
            if (SDK_INQ_MAX_DEVICES == rem_dev_num) {
                /* Max remote devices limit reached */
                break;
            }

            for (index = 0; index < rem_dev_num; index++) {
                if (!memcmp
                    (bd_addr, rem_bt_dev[index].bd_addr, BT_BD_ADDR_SIZE)) {
                    /* Remote device already noted */
                    flag = 1;
                    break;
                }
            }
            if (flag) {
                /* Remote device noted, so skip updation */
                break;
            }
            /* Store the remote device details */
            memcpy(rem_bt_dev[rem_dev_num].bd_addr, bd_addr, BT_BD_ADDR_SIZE);
            rem_bt_dev[rem_dev_num].page_scan_rep_mode = value_1;
            rem_bt_dev[rem_dev_num].clock_offset = value_2;
            /* Increase the count of number of devices discovered */
            rem_dev_num++;
        }
        break;
    case HCI_REMOTE_NAME_REQUEST_COMPLETE_EVENT:
        /* printf("Received HCI_REMOTE_NAME_REQUEST_COMPLETE_EVENT.\n"); */
        event_data += 1;
        /* Note the bd addr */
        bd_addr = event_data;
        event_data += 6;
        if (event_datalen > 7) {
            if ((event_datalen - 7) > SDK_REM_BT_DEV_NAME_MAX_LEN - 1) {
                /* Truncate the rem dev name length */
                rem_dev_name_len = SDK_REM_BT_DEV_NAME_MAX_LEN - 1;
            } else {
                rem_dev_name_len = event_datalen - 7;
            }

            /* Check if inquiry is in progress */
            if (TRUE == peer_initiated_acl_connection) {

                /**
                 * Not in inquiry, remote name request issued for a connected
                 * device, so get the sdk_status instance
                 */
                peer_initiated_acl_connection = FALSE;
                appl_get_status_instance_bd_addr(&dev_index, bd_addr);

                if (0 !=
                    strncmp((char *)event_data, (char *)SDK_REM_DEV_NAME_PREFIX,
                            strlen((char *)SDK_REM_DEV_NAME_PREFIX))) {
                    /* Disconnect ACL if the connection is requested form a
                     * Non-BlueMSP device */
                    retval =
                        BT_hci_disconnect(sdk_status[dev_index].
                                          acl_connection_handle, 0x13);
                    if (retval != API_SUCCESS) {
                        printf("** FAILED ** !! Reason Code = 0x%04X\n",
                               retval);
                    }
                }
                break;
            }

            /* Store the remote device name */
            bd_addr = rem_bt_dev[rem_dev_index].bd_addr;
            rem_dev_name = event_data;
            rem_dev_name[rem_dev_name_len] = '\0';

            printf("%02X %02X %02X %02X %02X %02X --> ", bd_addr[0], bd_addr[1],
                   bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            printf("%s\n", rem_dev_name);

            /* Check if "BlueMSP430Demo" is found */
            if (0 ==
                strncmp((char *)rem_dev_name, SDK_REM_DEV_NAME_PREFIX,
                        (strlen((char *)SDK_REM_DEV_NAME_PREFIX)))) {
                /* Allocate free spp instance if available */
                if (API_SUCCESS !=
                    appl_get_free_status_instance(&rem_bt_dev_index)) {
                    printf("No free SPP connection instance available\n");
                    break;
                }
                /* BlueMSP430Demo Found, so initiate connection */
                retval =
                    BT_hci_create_connection(bd_addr, SDK_CONFIG_ACL_PKT_TYPE,
                                             rem_bt_dev[rem_dev_index].
                                             page_scan_rep_mode, 0,
                                             rem_bt_dev[rem_dev_index].
                                             clock_offset, 0x01);
                /* If already connected */
                if (HCI_STATE_ALREADY_CONNECTED == retval) {
                    /* Initiate SDP Query */
                    appl_spp_sdp_query(rem_bt_dev_index);
                    SDK_SPP_CHANGE_STATE(rem_bt_dev_index, SDK_IN_SDP_QUERY);
                } else if (API_SUCCESS == retval) {
                    /* Populate the connection instance with bd_addr */
                    BT_mem_copy(sdk_status[rem_bt_dev_index].peer_bd_addr,
                                bd_addr, BT_BD_ADDR_SIZE);
                    /* On Success */
                    SDK_SPP_CHANGE_STATE(rem_bt_dev_index,
                                         SDK_IN_ACL_CONNECTION);
                }
                break;
            }
        }
        /* Start the query for next remote device discovered */
        rem_dev_index++;
        if (rem_dev_index < rem_dev_num) {
            /* Device exist for which name request has not been issued */
            retval =
                BT_hci_remote_name_request(rem_bt_dev[rem_dev_index].bd_addr,
                                           rem_bt_dev
                                           [rem_dev_index].page_scan_rep_mode,
                                           0x00,
                                           rem_bt_dev[rem_dev_index].
                                           clock_offset);
            if (retval != API_SUCCESS) {
                printf("Remote Name Request FAILED !! Error Code = 0x%04x\n",
                       retval);
            }
        } else if (rem_dev_index == rem_dev_num) {
            /* Restart Inquiry as BlueMSP430Demo device is not found */
            retval =
                BT_hci_inquiry(SDK_INQUIRY_LAP, SDK_INQUIRY_LEN,
                               SDK_NUM_RESPONSES);
            if (retval != API_SUCCESS) {
                sdk_display(SDK_MSG_AREA,
                            (const UCHAR *)"Failed to initiate Inquiry\n", 0);
            } else {
                rem_dev_index = rem_dev_num = 0;
                sdk_display(SDK_MSG_AREA, (const UCHAR *)
                            "Inquiry Started...Wait for Completion\n", 0);
            }
        }
        break;
    default:
        /* sdk_display(SDK_STATUS_AREA,"Unknown Event Code 0x%02X Received.\n",
         * event_type); */
        break;
    }

    return API_SUCCESS;
}

/**
 * \brief   Function to dump a buffer byte stream onto the SDK display
 * \param   buffer      Pointer to the buffer to be displayed
 * \param   length      Length of the buffer
 * \return  none
 * \see     appl_hci_cb.c
 */
void appl_dump_bytes(UCHAR * buffer, UINT16 length)
{
    char hex_stream[49];
    char char_stream[17];
    UINT32 i;
    UINT16 offset, count;
    UCHAR c;
    sdk_display(SDK_MSG_AREA, (const UCHAR *)"\n", 0);
    /* sdk_display(SDK_STATUS_AREA,"-- Dumping %d Bytes --\n", */
    /* (int)length); */
    sdk_display(SDK_MSG_AREA, (const UCHAR *)
                "-------------------------------------------------------------------\n", 0);
    count = 0;
    offset = 0;
    for (i = 0; i < length; i++) {
        c = buffer[i];
        /* sdk_display(SDK_STATUS_AREA,hex_stream + offset, "%02X ", c); */
        if ((c >= 0x20) && (c <= 0x7E)) {
            char_stream[count] = c;
        } else {
            char_stream[count] = '.';
        }

        count++;
        offset += 3;
        if (count == 16) {
            char_stream[count] = '\0';
            count = 0;
            offset = 0;
            /* sdk_display(SDK_STATUS_AREA,"%s %s\n", */
            /* hex_stream, char_stream); */
            memset(hex_stream, 0, 49);
            memset(char_stream, 0, 17);
        }
    }

    if (offset != 0) {
        char_stream[count] = '\0';
        /* Maintain the alignment */
        /* sdk_display(SDK_STATUS_AREA,"%-48s %s\n", */
        /* hex_stream, char_stream); */
    }

    sdk_display(SDK_MSG_AREA, (const UCHAR *)
                "-------------------------------------------------------------------\n", 0);
    sdk_display(SDK_MSG_AREA, (const UCHAR *)"\n", 0);
}


/**
 * @brief   Callback function for SM event notification
 * @param   event_type Type of event
 * @param   bd_addr Address of peer device
 * @param   event_data Data buffer for events
 * @return  API_SUCCESS/API_FAILURE
 * @see     appl_sm_cb.c
 */

API_RESULT sdk_sm_ui_notify_cb(UCHAR event_type, UCHAR * bd_addr,
                               UCHAR * event_data)
{
#ifdef BT_SSP
    UINT32 numeric_val;
    UCHAR user_reply;
#endif /* BT_SSP */

    API_RESULT retval;
    UCHAR reason, flag;
    UCHAR link_key[BT_LINK_KEY_SIZE];

    UCHAR msg_string[100];

    retval = API_SUCCESS;

    switch (event_type) {
    case SM_ACL_CONNECT_REQUEST_NTF:
        sprintf((char *)msg_string,
                "Received UI Connection Request from SM\n"
                "BD_ADDR = %02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0],
                bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);

        sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);

        /* Accept the incoming connection */
        reason = 0x0;

        /* Automatically accepting - in this sample example */

        /* 
         * Application developer might choose to inform the user
         * about an incomming connection.
         *
         * Application should not call the user notification callback
         * from this SM callback context, that will block the entire
         * read task, which will prevent handling of data recevied
         * from the transport and might have unpredictable behaviour.
         */
        retval = BT_sm_connection_request_reply(bd_addr, 0x01, reason);

        if (retval != API_SUCCESS) {
            sdk_display(SDK_MSG_AREA, (const UCHAR *)
                        "Replying to UI Connection Request ...  FAILED", 0);

            break;
        } else {
            sdk_display(SDK_MSG_AREA, (const UCHAR *)"OK\n", 0);
        }

        break;

    case SM_PIN_CODE_REQUEST_NTF:

        sprintf((char *)msg_string,
                "Received UI PIN Code Request from SM\n"
                "BD_ADDR = %02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0],
                bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);

        sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);

        sdk_display(SDK_MSG_AREA,
                    (const UCHAR *)"Replying to UI PIN Code Request (+Ve) ... ",
                    0);

        /* 
         * Application developer has to provide the user interface to
         * get the PIN code from user.
         *
         * For the demonstration we are using the fixed PIN code.
         *
         * Application should not call the user notification callback
         * from this SM callback context, that will block the entire
         * read task, which will prevent handling of data recevied
         * from the transport and might have unpredictable behaviour.
         *
         */
        retval =
            BT_sm_pin_code_request_reply(bd_addr, (UCHAR *) SDK_CONFIG_PIN,
                                         strlen(SDK_CONFIG_PIN)
            );

        if (retval != API_SUCCESS) {
            sprintf((char *)msg_string,
                    (const char *)"FAILED ! Reason = 0x%04X\n", retval);

            sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);
            break;
        } else {
            sdk_display(SDK_MSG_AREA, (const UCHAR *)"OK\n", 0);
        }

        break;

    case SM_LINK_KEY_REQUEST_NTF:
        sprintf((char *)msg_string,
                "Received UI Link Key Request from SM\n"
                "BD_ADDR = %02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0],
                bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);

        sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);

        /* Get Link Key from Device Database */
        retval = BT_sm_get_device_link_key(bd_addr, link_key);
        flag = (retval == API_SUCCESS) ? 0x1 : 0x0;

        sprintf((char *)msg_string, "Replying to UI Link Key Request (%s) ... ",
                (retval == API_SUCCESS) ? "+Ve" : "-Ve");

        sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);
        retval = BT_sm_link_key_request_reply(bd_addr, link_key, flag);

        if (retval != API_SUCCESS) {
            sprintf((char *)msg_string, "FAILED ! Reason = 0x%04X\n", retval);

            sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);
            break;
        } else {
            sdk_display(SDK_MSG_AREA, (const UCHAR *)"OK\n", 0);
        }

        break;

    case SM_USER_CONF_REQUEST_NTF:
        sprintf((char *)msg_string,
                "Received UI User Conf Request from SM\n"
                "BD_ADDR = %02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0],
                bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);

        sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);


        if ((SDK_IO_CAPABILITY == SM_IO_CAPABILITY_NO_INPUT_NO_OUTPUT)
            || (SDK_IO_CAPABILITY == SM_IO_CAPABILITY_DISPLAY_ONLY)) {
            /* Device has no Input Capability, so directly accept the
             * connection */
            retval = BT_sm_user_conf_request_reply(bd_addr, 0x01);
            if (retval != API_SUCCESS) {
                sprintf((char *)msg_string,
                        (const char *)"FAILED ! Reason = 0x%04X\n", retval);

                sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);
            } else {
                sdk_display(SDK_MSG_AREA, (const UCHAR *)"OK\n", 0);
            }
        } else {

            /* Get Numeric Value */
            numeric_val = event_data[3];
            numeric_val <<= 8;
            numeric_val |= event_data[2];
            numeric_val <<= 8;
            numeric_val |= event_data[1];
            numeric_val <<= 8;
            numeric_val |= event_data[0];

            sprintf((char *)msg_string, "Numeric Value = 0x%08X\n",
                    (unsigned int)numeric_val);

            sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);

            /* 
             * The User confirmation is done automatically here.
             * But application developer can display this numeric value to the user
             * and read his response (accept/reject) 
             */
            sdk_display(SDK_MSG_AREA,
                        (const UCHAR *)"Replying to UI User Conf Request ... ",
                        0);

            user_reply = 0x01;
            retval = BT_sm_user_conf_request_reply(bd_addr, user_reply);
            if (retval != API_SUCCESS) {
                sprintf((char *)msg_string,
                        (const char *)"FAILED ! Reason = 0x%04X\n", retval);

                sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);
            } else {
                sdk_display(SDK_MSG_AREA, (const UCHAR *)"OK\n", 0);
            }
        }
        break;

    case SM_USER_PASSKEY_REQUEST_NTF:
        sprintf((char *)msg_string,
                "Received UI User Passkey Request from SM\n"
                "BD_ADDR = %02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0],
                bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);

        sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);
        /* 
         * Application developer has to provide HMI interface for entering the
         * pass key which is notified on the peer device using 
         * SM_USER_PASSKEY_NTF event on its side. The pass key has to be replied  
         * using BT_sm_user_passkey_request_reply API.
         * [NOTE: Local device has to send the Passkey entry started event,
         *        Passkey digit entered event, Passkey digit erased event,
         *        Passkey cleared event or Passkey entry completed event
         *        when it recieves respective events from HMI using the API
         *        BT_hci_send_keypress_notification.
         */

        sdk_display(SDK_MSG_AREA, (const UCHAR *)
                    "Please Reply to User Passkey Request using HMI.\n", 0);
        break;

    case SM_USER_PASSKEY_NTF:
        sprintf((char *)msg_string,
                "Received UI User Passkey Notification from SM\n"
                "BD_ADDR = %02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0],
                bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);

        sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);

        /* Get Numeric Value */
        numeric_val = event_data[3];
        numeric_val <<= 8;
        numeric_val |= event_data[2];
        numeric_val <<= 8;
        numeric_val |= event_data[1];
        numeric_val <<= 8;
        numeric_val |= event_data[0];

        /* 
         * Application developer has to display this passkey, this passkey value 
         * has to be supplied by the remote device upon notifying with 
         * SM_USER_PASSKEY_REQUEST_NTF event on its side.
         */
        sprintf((char *)msg_string, "Numeric Value = %u (0x%08X)\n",
                (unsigned int)numeric_val, (unsigned int)numeric_val);

        sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);

        break;

    default:
        sprintf((char *)msg_string, "*** Unknown/Undefined Event Type 0x%02X\n",
                event_type);

        sdk_display(SDK_MSG_AREA, (const UCHAR *)msg_string, 0);
        break;
    }

    return retval;
}
