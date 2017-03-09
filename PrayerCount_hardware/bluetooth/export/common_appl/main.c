
/**
 * Copyright (c) 2009 MindTree Ltd.  All rights reserved.
 * \file    main.c
 * \brief   This file contains 'main()' entry point of the
 *          application and low_level_init function
 */


/* Header File Inclusion */
#include "appl_sdk.h"

/* Extern Function declarations */
/* Function to initialize the allocated buffers */
extern void sdk_buffer_mgmt_init(void);


/* Functions */

/**
 * \fn      main
 * \brief   Entry point to application. Initializes BSP, OS BT EtherMind stack
 * \param   none
 * \return  none
 */

int main(void)
{
    API_RESULT retval;

    sdk_init_bsp();             /* Initialize the peripherals on the board */

    sdk_buffer_mgmt_init();     /* Initialises the free buffer pools */

    BT_os_init();               /* Initialize Bluetooth */

    sdk_init_ui();              /* Initialize user interface menu */

    BT_ethermind_init();        /* Configure the EtherMind stack */

    sdk_appl_init();            /* Initialize the sdk status flags */
/*
    while (1){
       UCHAR appl_accelerometer_buffer[6];

        sdk_accelerometer_read(appl_accelerometer_buffer,sizeof(appl_accelerometer_buffer));
        halUsbSendString(appl_accelerometer_buffer,sizeof(appl_accelerometer_buffer));
//        for (int i=0;i<0XFFFF;i++)
          for (int j=0;j<0X0FF;j++);

    }*/
    retval = BT_spp_init(appl_spp_notify_cb);   /* Init SPP */

    if (API_SUCCESS != retval) {
        printf("Failed to initialize SPP, reason %04X\n", retval);
    }

    sdk_start_scheduler();      /* Start OS scheduler */

    return 0;                   /* Should never reach here */
}


/**
 * \fn      __low_level_init
 * \brief   The function __low_level_init is called by the start-up code
 *          before doing the normal initialization of data segments.
 *          If the return value is zero, initialization is not
 *          performed. In the run-time library there is a dummy __low_level_init, which
 *          does nothing but return 1. This means that the start-up routine proceeds
 *          with initialization of data segments. To replace this dummy, compile a
 *          customized version (like the example below) and link it with the rest of
 *          your code.
 * \param   void
 * \return  1
 */

#include "msp430x54xA.h"
int __low_level_init(void)
{
    WDTCTL = WDTPW + WDTHOLD;

    /* Choose if segment initialization */
    /* should be done or not.  */
    /* Return: 0 to omit seg_init */
    /* 1 to run seg_init */

    return (1);
}
