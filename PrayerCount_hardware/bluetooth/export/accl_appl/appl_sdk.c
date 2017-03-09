
/**
 * Copyright (c) 2009 MindTree Ltd.  All rights reserved.
 * \file    appl_sdk.c
 * \brief   This file contains initialization,
 *          BT menu handlers and connection related functions
 */


/* Header File Inclusion */
#include "appl_sdk.h"

/* Extern Variables */

/* User selected remote BT device info */
extern UCHAR rem_dev_num, rem_dev_index;

/* Static Variables */
static UCHAR appl_accelerometer_buffer[4];

/* Global Variables */
/* spp connections status information */
SDK_SPP_CONNECTION_STATUS sdk_status[SPP_MAX_ENTITY];

UCHAR sdk_initiator = FALSE;

/* Bluetooth Power On/Off Status Flag */
UCHAR sdk_bt_power = SDK_BT_OFF;

/* Bluetooth Discoverable On/Off Status Flag */
UCHAR sdk_bt_visible = SDK_DISC_OFF;

/* Flag to indicated that ACL connection is initiated by peer */
UCHAR peer_initiated_acl_connection = FALSE;

/* Flag to indicate that SPP connection is in progress */
UCHAR sdk_connect_in_progress = FALSE;

/* Functions */

/**
 * @brief   Function to initialize the sdk status flags
 * @param   none
 * @return  none
 * @see     appl_hci_cb.c
 */
void sdk_appl_init(void)
{
    UCHAR index;

    /* Bluetooth Power On/Off Status Flag */
    sdk_bt_power = SDK_BT_OFF;

    /* Bluetooth Discoverable On/Off Status Flag */
    sdk_bt_visible = SDK_DISC_OFF;

    for (index = 0; index < SPP_MAX_ENTITY; index++) {
        /* Initialize the spp connection state */
        SDK_SPP_CHANGE_STATE(index, SDK_DISCONNECTED);
        /* Initialize the spp data transfer state */
        SDK_SPP_CHANGE_TX_STATE(index, SDK_SPP_TX_OFF);
#ifdef SDK_ENABLE_SNIFF_MODE
        /* Change link state */
        SDK_SPP_CHANGE_LINK_STATE(index, SDK_OFF);
#endif /* SDK_ENABLE_SNIFF_MODE */
        /* Change data to be sent flag state */
        SDK_SPP_CHANGE_DATA_STATE(index, FALSE);
    }
}

/**
 * @brief   Handles the Bluetooth menu options
 * @param   input   menu option selected
 * @return  none
 * @see     appl_menu_pl.c
 */
void sdk_bluetooth_menu_handler(UCHAR input)
{
    API_RESULT retval;

    /**
     * Only option will be SPP Connect, Which initiates Inquiry looks for
     * remote BT Device with "BlueMSP430Demo" name and connects with it.
     */
    switch (input) {
    case OP_PEER_CONNECT:      /* Connect/Disconnect Button Pressed */
        if (SDK_IS_BT_POWERED_ON()) {
            /* Check if SPP is already connected */
            if (SDK_IS_DISCONNECTED(0)) {
                /* SPP not connected, so initiate connection */
                /* Set the initiator flag */
                sdk_initiator = TRUE;
                /* SPP Connection started, set the flag */
                sdk_connect_in_progress = TRUE;
                /* Initiate Inquiry */
                retval =
                    BT_hci_inquiry(SDK_INQUIRY_LAP, SDK_INQUIRY_LEN, SDK_NUM_RESPONSES);
                if (retval != API_SUCCESS) {
                    sdk_display(SDK_MSG_AREA,
                                (const UCHAR *)"Failed to initiate Inquiry\n",
                                0);
                } else {
                    rem_dev_index = rem_dev_num = 0;
                    sdk_display(SDK_MSG_AREA, (const UCHAR *)
                                "Inquiry Started...Wait for Completion\n", 0);
                }
            }            
            else if (SDK_IS_SPP_CONNECTED(0))
            {
              /* SPP connected, so initiate disconnection */
#ifdef SDK_ENABLE_SNIFF_MODE
                /* Check if sniff mode is active */
                if (SDK_IS_IN_SNIFF_MODE(0)) {
                    /* Exit the sniff mode for disconnection */
                    retval =
                        BT_hci_exit_sniff_mode(sdk_status[0].
                                               acl_connection_handle);
                    if (retval != API_SUCCESS) {
                      sdk_display(SDK_MSG_AREA, (const UCHAR *)
                        "Exit sniff mode attempt failed\n", 0);
                    } else {
                        sdk_display(SDK_MSG_AREA, (const UCHAR *)
                          "Exiting sniff mode\n", 0);
                        
                        /* Change spp connection state */
                        SDK_SPP_CHANGE_STATE (0, SDK_IN_SPP_DISCONNECTION);
                        break;
                    }
                }
#endif /* SDK_ENABLE_SNIFF_MODE */
                /* Initiate Disconnection */
                appl_spp_disconnect(0);

                /* On Success */
                SDK_SPP_CHANGE_STATE(0, SDK_IN_SPP_DISCONNECTION);              
            }                          
        }
        break;
    case OP_PEER_DATASEND:
    {
        /* Dtasend stop and pause toggle button presses */
        if (SDK_IS_SPP_CONNECTED(0)) {
            if (SDK_IS_SPP_TX_STARTED(0)) {
                /* Stop Sending Data */
                SDK_SPP_CHANGE_TX_STATE(0, SDK_SPP_TX_OFF);

            } else {
                /* Start Sending Data */
                SDK_SPP_CHANGE_TX_STATE(0, SDK_SPP_TX_ON);
                appl_acceleromenter_read_spp_send(0);
            }

        } else {
            sdk_display(SDK_MSG_AREA, (const UCHAR *)"SPP is not connected\n",
                        0);
        }
    }
        break;
    }

}

/**
 * @brief   Function to handle BT on complete event.
 * 		    Application can perform specific profile initialization,
 * 		    setting various bluetooth controller parameters like
 * 		    Class Of Device, default link supervision timeout etc.
 *		    For TI processor the init script also has to be sent.
 * @param   none
 * @return  none
 * @see     appl_bt_on_cb.c
 */
void appl_bluetooth_on_complete_event_handler(void)
{
    API_RESULT retval;

    sdk_display(SDK_MSG_AREA, (const UCHAR *)"Bluetooth is Powered On\n", 0);

    /* Write Inquiry Scan mode (Interlaced scanning) */
    retval = BT_hci_write_inquiry_scan_type(0x01);
    if (retval != API_SUCCESS) {
        sdk_display(SDK_MSG_AREA,
                    (const UCHAR *)"Failed to turn set inquiry scan type\n", 0);
    }

    /* Start the SPP Profile */
    appl_spp_start();
}

/**
 * @brief   Function to handle ACL connection complete event
 * @param   bd_addr BT address
 * @param   status Status of the connection
 * @param   connection_handle Handle for the connection
 * @return  none
 * @see     appl_hci_cb.c
 */
void appl_acl_connection_complete_event(UCHAR * bd_addr, UCHAR status,
                                        UINT16 connection_handle)
{
    UCHAR dev_index;
    API_RESULT retval;

    /* Check if the connection is initiated from local device */
    /* If locally initiated, check if the status is success */
    if (API_SUCCESS == appl_get_status_instance_bd_addr(&dev_index, bd_addr)) {
        /* If Success, Initiate SDP Query */
        if (0x00 == status) {
                if (SDK_IS_IN_ACL_CONNECTION(dev_index)) {            
                /* Store the ACL Handle */
                sdk_status[dev_index].acl_connection_handle = connection_handle;

#ifdef SDK_ENABLE_SNIFF_MODE
                SDK_SPP_CHANGE_LINK_STATE(dev_index, SDK_ACTIVE)
#endif /* SDK_ENABLE_SNIFF_MODE */
                    /* Start the SDP Query */
                    appl_spp_sdp_query(dev_index);

                SDK_SPP_CHANGE_STATE(dev_index, SDK_IN_SDP_QUERY);
            }

        }
    } else {
        /* Peer initiated acl connection, if success, note the connection info */
        if (0x00 == status) {
            /* Remote Device initiated connection, suffice if resources are
             * available */
            appl_get_free_status_instance(&dev_index);
            BT_mem_copy(sdk_status[dev_index].peer_bd_addr, bd_addr,
                        BT_BD_ADDR_SIZE);

            sdk_status[dev_index].acl_connection_handle = connection_handle;
            SDK_SPP_CHANGE_STATE(dev_index, SDK_ACL_CONNECTED);
            SDK_SPP_CHANGE_LINK_STATE(dev_index, SDK_ACTIVE);
            retval =
                BT_hci_remote_name_request(sdk_status[dev_index].peer_bd_addr,
                                           0x00, 0x00, 0x00);

            if (retval != API_SUCCESS) {
                peer_initiated_acl_connection = FALSE; 
                printf("Remote Name Request FAILED !! Error Code = 0x%04x\n",
                       retval);
            }
        }
    }
}

/**
 * @brief   Function to read from accelerometer and send over SPP
 * @param   dev_index Index used to denote the device
 * @return  none
 * @see     appl_hci_cb.c, appl_spp.c
 */
int sum;
int pos_x_tmp,pos_y_tmp,pos_z_tmp;
int pos_x,pos_y,pos_z;
int max = 300;

void appl_acceleromenter_read_spp_send(UCHAR dev_index)
{
//    sdk_accelerometer_read(appl_accelerometer_buffer,
//                           sizeof(appl_accelerometer_buffer)
//        );




    
    sdk_accelerometer_read_int_xyz(&pos_x,&pos_y,&pos_z);
    
//           for (int i=0;i<0XFFFF;i++)
//          for (int j=0;j<0XFFF;j++);

/*    if (pos_x_tmp - pos_x > max || pos_x_tmp - pos_x < -max){
          appl_spp_write(dev_index, "X",1);
          pos_x_tmp = pos_x;
    }else 
    if (pos_y_tmp - pos_y > max || pos_y_tmp - pos_y < -max){
          appl_spp_write(dev_index, "Y",1);
          pos_y_tmp = pos_y;
    } else
    if (pos_z_tmp - pos_z > max || pos_z_tmp - pos_z < -max){
          appl_spp_write(dev_index, "Z",1);
          pos_z_tmp = pos_z;
    } else 
          appl_spp_write(dev_index, ".",1);
*/
    
    /*appl_spp_write(dev_index, pos_x /256 ,1);
    appl_spp_write(dev_index, pos_x  ,1);
    
        *int_accl_z = (0xFF - pos_x) * 10;
    	*int_accl_z = 0xFFFF - pos_x;*/

}

/**
 * @brief   Function to get free instance in sdk_status_array
 * @param   id [OUT] parameter containing index of unused connection instance
 * @return  API_success/API_failure
 * @see     appl_hci_cb.c, appl_menu_pl.c, sdk_pl_accelerometer.c
 */
API_RESULT appl_get_free_status_instance(UCHAR * id)
{
    UCHAR index;

    for (index = 0; index < SPP_MAX_ENTITY; index++) {
        if (SDK_IS_DISCONNECTED(index)) {
            *id = index;
            return API_SUCCESS;
        }
    }
    printf("No more free SPP Connection instances\n");
    return API_FAILURE;
}

/**
 * @brief   Function to get spp connection instance based on acl connection handle
 * @param   id [OUT] paramter containing instance id corresponding to the handle
 * @param   acl_connection_handle Connection handle for ACL connection
 * @return  API_success/API_failure
 * @see     appl_hci_cb.c
 */
/* Function to get spp connection instance based on acl connection handle */
API_RESULT appl_get_status_instance_acl(UCHAR * id,
                                        UINT16 acl_connection_handle)
{
    UCHAR index;

    for (index = 0; index < SPP_MAX_ENTITY; index++) {
        if ((acl_connection_handle == sdk_status[index].acl_connection_handle)
            && (!SDK_IS_DISCONNECTED(index))) {
            *id = index;
            return API_SUCCESS;
        }
    }
    printf
        ("No Such SPP Connection instance available for this acl connection handle %04X\n",
         acl_connection_handle);
    return API_FAILURE;
}

/**
 * @brief   Function to get spp connection instance based on spp connection handle
 * @param   id [OUT] paramter containing instance id corresponding to the handle
 * @param   acl_connection_handle Connection handle for SPP connection
 * @return  API_success/API_failure
 * @see     appl_spp.c
 */
API_RESULT appl_get_status_instance_spp(UCHAR * id,
                                        UINT16 spp_connection_handle)
{
    UCHAR index;

    for (index = 0; index < SPP_MAX_ENTITY; index++) {
        if ((spp_connection_handle == sdk_status[index].spp_connection_handle)
            && (!SDK_IS_DISCONNECTED(index))) {
            *id = index;
            return API_SUCCESS;
        }
    }
    printf
        ("No Such SPP Connection instance available for this SPP handle %04X\n",
         spp_connection_handle);
    return API_FAILURE;
}

/**
 * @brief   Function to get spp connection instance based on peer bd address
 * @param   id [OUT] paramter containing instance id corresponding to the address
 * @param   rem_bd_addr BT address of device to be found
 * @return  API_success/API_failure
 * @see     appl_hci_cb.c, appl_spp.c
 */
API_RESULT appl_get_status_instance_bd_addr(UCHAR * id, UCHAR * rem_bd_addr)
{
    UCHAR index;

    for (index = 0; index < SPP_MAX_ENTITY; index++) {
        if (!memcmp(sdk_status[index].peer_bd_addr, rem_bd_addr, 6)
            && (!SDK_IS_DISCONNECTED(index))) {
            *id = index;
            return API_SUCCESS;
        }
    }
    if (index == SPP_MAX_ENTITY) {
        /* printf ("No Such SPP Connection instance available\n"); */
    }
    return API_FAILURE;
}
