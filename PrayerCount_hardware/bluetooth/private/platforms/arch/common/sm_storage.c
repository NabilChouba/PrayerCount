
/**
 *  @file sm_storage.c
 *
 *  This file contains the implementation for all the storage related APIs.
 *  This implementation assumes that a file system will be available on the
 *  target device. In cases where target device does not have a filesystem
 *  (eg.  headset), we will have to replace this file with the target specific
 *  implementation.
 *
 *  Version: Windows User Mode
 */

/* 
 *  Copyright (C) 2000-2010. MindTree Ltd.
 *  All rights reserved.
 */

/** ----------------------------------------- Header File Inclusion */
#include "sm_internal.h"
#include "sm_extern.h"

#ifdef SM_STORAGE

/** ----------------------------------------- Global Definitions */

/** Persistent Storage Write */
#define sm_ps_write(p, nb)

/** Persistent Storage Read */
#define sm_ps_read(p, nb)


/** ----------------------------------------- External Global Variables */


/** ----------------------------------------- Exported Global Variables */


/** ----------------------------------------- Static Global Variables */


/** ----------------------------------------- Functions */

/**
 *  This function implements writing into SM's persistent storage media.
 *  If the media is not found then it attempts to create one. On successful
 *  opening (creation) of the storage device, it writes SM's current
 *  configuration details (Default Security Mode, Encryption Mode, Connection
 *  Accept, PIN Code and Authorization flags), followed by the list of trusted
 *  devices with their PIN Code and Link Keys.
 *
 *  @return  none
 */
void sm_storage_write(void)
{
    /* Commenting the code related to FILE operations, to reduce RAM utilization */  
#if 0
    FILE *storage_fd;
#endif
    UCHAR i, j, n_trusted, pin_val, pin_len, trust, rank;

    SM_TRC(bt_debug_fd,
           "[SM] Storage Write: Opening Persistent Storage Device\n");
#if 0
    /* Open Storage Device */
    storage_fd = fopen(SM_STORAGE_MEDIA, "wb");
    if (storage_fd == NULL) {
        SM_ERR(bt_debug_fd,
               "[SM] Storage Write: FAILED to Open Persistent Storage Device\n");

        return;
    }
#endif

    SM_TRC(bt_debug_fd,
           "[SM] Storage Write: Writing into Persistent Storage Device\n");

#ifndef BT_SSP
    /* Default Security Mode */
    sm_ps_write(&sm_security_mode, 1);

    /* Default Encryption Mode */
    sm_ps_write(&sm_encryption_mode, 1);
#endif /* BT_SSP */

    /* Default PIN Length */
    sm_ps_write(&sm_default_pin_length, 1);

    /* Default PIN */
    sm_ps_write(sm_default_pin, BT_PIN_CODE_SIZE);

    pin_val = 0x0;
    if (BT_PIN_CODE_SIZE < 16) {
        pin_len = 16 - BT_PIN_CODE_SIZE;
        for (i = 0; i < pin_len; i++) {
            sm_ps_write(&pin_val, 1);
        }
    }

    /* Find number of Trusted/Paired Devices */
    n_trusted = 0;
    for (i = 0; i < SM_MAX_DEVICES; i++) {
        if ((sm_devices[i].valid != SM_DEVICE_INVALID)
            && ((SM_IS_DEVICE_TRUSTED(i) == SM_TRUE)
                || (SM_IS_DEVICE_LINK_KEY_AVAILABLE(i) == SM_TRUE))) {
            n_trusted++;
        }
    }

    SM_TRC(bt_debug_fd, "[SM] Storage Write: Number of Trusted Device = %u\n",
           n_trusted);

    /* Write Number of Trusted/Paired Devices */
    sm_ps_write(&n_trusted, 1);

    /* Write Information for Trusted/Paired Devices */
    if (n_trusted > 0) {
        for (i = 0; i < SM_MAX_DEVICES; i++) {
            if ((sm_devices[i].valid != SM_DEVICE_INVALID)
                && ((SM_IS_DEVICE_TRUSTED(i) == SM_TRUE)
                    || (SM_IS_DEVICE_LINK_KEY_AVAILABLE(i) == SM_TRUE))) {
                SM_TRC(bt_debug_fd,
                       "[SM] Storage Write: Trusted/Paired BD_ADDR "
                       "%02X:%02X:%02X:%02X:%02X:%02X\n",
                       sm_devices[i].bd_addr[0], sm_devices[i].bd_addr[1],
                       sm_devices[i].bd_addr[2], sm_devices[i].bd_addr[3],
                       sm_devices[i].bd_addr[4], sm_devices[i].bd_addr[5]);

                /* BD_ADDR */
                sm_ps_write(sm_devices[i].bd_addr, BT_BD_ADDR_SIZE);

                /* Link Key */
                sm_ps_write(sm_devices[i].link_key, BT_LINK_KEY_SIZE);
                sm_ps_write(&sm_devices[i].link_key_type, 1);

                /* Trust Flag */
                trust = (SM_IS_DEVICE_TRUSTED(i) == SM_TRUE) ? 0x01 : 0x00;
                sm_ps_write(&trust, 1);

                /* Device Name */
                sm_ps_write(&sm_devices[i].name_length, 1);
                sm_ps_write(sm_devices[i].name, SM_DEVICE_NAME_SIZE);

                /* Device PIN Code */
                sm_ps_write(&sm_devices[i].pin_length, 1);
                sm_ps_write(sm_devices[i].pin_code, BT_PIN_CODE_SIZE);
                if (BT_PIN_CODE_SIZE < 16) {
                    pin_len = 16 - BT_PIN_CODE_SIZE;
                    for (j = 0; j < pin_len; j++) {
                        sm_ps_write(&pin_val, 1);
                    }
                }

                /* Platform Specific Attribute */
                rank = (UCHAR) sm_devices[i].device_attr_pl;
                sm_ps_write(&rank, 1);
            }
        }
    }
#if 0
    /* Close the device */
    fclose(storage_fd);
#endif
}


/**
 *  This function implements reading from SM's persistent storage media.
 *  On successful opening of the storage device, it reads SM's current
 *  configuration details (Default Security Mode, Encryption Mode, Connection
 *  Accept, PIN Code and Authorization flags), followed by the list of trusted
 *  devices with their PIN Code and Link Keys.
 *
 *  @return  none
 */
void sm_storage_read(void)
{
#if 0
    FILE *storage_fd;
#endif
    UCHAR attr;
    UCHAR di, trust, n_trusted, name_length, pin_length, link_key_type;
    UCHAR bd_addr[BT_BD_ADDR_SIZE];
    UCHAR name[SM_DEVICE_NAME_SIZE];
    UCHAR link_key[BT_LINK_KEY_SIZE];
    UCHAR pin_code[16];

    SM_TRC(bt_debug_fd,
           "[SM] Storage Read: Opening Persistent Storage Device\n");
#if 0
    /* Open Storage Device */
    storage_fd = fopen(SM_STORAGE_MEDIA, "rb");
    if (storage_fd == NULL) {
        SM_ERR(bt_debug_fd,
               "[SM] Storage Read: FAILED to Open Persistent Storage Device\n");

        return;
    }
#endif

    SM_TRC(bt_debug_fd,
           "[SM] Storage Read: Reading from Persistent Storage Device\n");

#ifndef BT_SSP
    /* Default Security Mode */
    sm_ps_read(&sm_security_mode, 1);

    /* Default Encryption Mode */
    sm_ps_read(&sm_encryption_mode, 1);
#endif /* BT_SSP */

    /* Default PIN Length */
    sm_ps_read(&sm_default_pin_length, 1);

    /* Default PIN */
    sm_ps_read(sm_default_pin, BT_PIN_CODE_SIZE);

    if (BT_PIN_CODE_SIZE < 16) {
        /* Dummy */
        sm_ps_read(pin_code, (16 - BT_PIN_CODE_SIZE));
    }

    /* Read Number of Trusted Devices */
    sm_ps_read(&n_trusted, 1);

    SM_TRC(bt_debug_fd, "[SM] Storage Read: Number of Trusted Device = %u\n",
           n_trusted);

    /* Read Information for Trusted/Paired Devices */
    while (n_trusted > 0) {
        sm_ps_read(bd_addr, BT_BD_ADDR_SIZE);
        sm_ps_read(link_key, BT_LINK_KEY_SIZE);
        sm_ps_read(&link_key_type, 1);
        sm_ps_read(&trust, 1);
        sm_ps_read(&name_length, 1);
        sm_ps_read(name, SM_DEVICE_NAME_SIZE);
        sm_ps_read(&pin_length, 1);
        sm_ps_read(pin_code, 16);
        sm_ps_read(&attr, 1);

        SM_TRC(bt_debug_fd,
               "[SM] Storage Read: Trusted/Paired BD_ADDR "
               "%02X:%02X:%02X:%02X:%02X:%02X\n", bd_addr[0], bd_addr[1],
               bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);

        /* Populate the read device in the devices table glob var */
        di = (UCHAR) sm_create_device_entity(bd_addr);
        if (SM_MAX_DEVICES == di) {
            break;
        }

        /* Copy Name for the Device */
        BT_mem_copy(sm_devices[di].name, name, SM_DEVICE_NAME_SIZE);
        sm_devices[di].name_length = name_length;

        /* Copy Link Key for the Device */
        BT_mem_copy(sm_devices[di].link_key, link_key, BT_LINK_KEY_SIZE);
        sm_devices[di].link_key_type = link_key_type;
        SM_SET_DEVICE_LINK_KEY_AVAILABLE(di, SM_TRUE);

        /* Set Device as Trusted */
        if (trust == 0x1) {
            SM_SET_DEVICE_TRUSTED(di, SM_TRUE);
        }

        /* Copy PIN Code for the Device */
        sm_devices[di].pin_length = 0;
        memset(sm_devices[di].pin_code, 0x0, BT_PIN_CODE_SIZE);
        if ((pin_length > 0) && (pin_length <= BT_PIN_CODE_SIZE)) {
            sm_devices[di].pin_length = pin_length;
            BT_mem_copy(sm_devices[di].pin_code, pin_code, pin_length);
        }

        /* Set Device Valid Flag */
        sm_devices[di].valid = SM_DEVICE_USED;

        /* Set Platform specific Device Attribute */
        sm_set_device_attr_pl(di, (SM_DEVICE_ATTR_PL) attr);

        n_trusted--;
    }
#if 0
    /* Close the device */
    fclose(storage_fd);
#endif
}

#endif /* SM_STORAGE */
