
/**
 * Copyright (c) 2009 MindTree Ltd.  All rights reserved.
 * \file    sdk_pl.c
 * \brief   This file contains platform specific function definitions
 */

/* Header File Inclusion */
#include "sdk_pl.h"
#include "BT_queue.h"
#include "task.h"
#include "appl_sdk.h"
#include "appl_bt_rf.h"
#include "hal_I2C.h"
#include "msp430_uart.h"

/* external Variables */

#ifdef MSP430_LPM_ENABLE
extern UCHAR lpm_mode;
#endif /* MSP430_LPM_ENABLE */
extern UCHAR uart_data;
extern DATA_RX_QUEUE data_rx_queue;
extern UCHAR temp_header_buffer[MAX_PKT_HDR_LEN];
extern UCHAR temp_header_buffer_idx;

extern UCHAR circular_buf[MAX_CIRCULAR_BUF_SIZE];
extern INT16 circular_buf_wt;
extern INT16 circular_buf_rd;

extern volatile UINT32 inactivity_counter;
extern char accl_x, accl_y, accl_z;

/* Bluetooth Power On/Off Status Flag */
extern UCHAR sdk_bt_power;
extern const UINT32 uart_clk[];


/* Global Variables */
/* Define threshold to distinguish between Positive and Negative Acceleration */
const char ACCL_POS_NEG_TH = 128;
/* Define offset for accelerometer value for scaling purposes */
const int ACCL_SCALE_OFFSET = 2048;

/* Flag to check if USB is connected */
UCHAR sdk_usb_detected = FALSE;

/* variable to store LED status before entering LPM */
UCHAR LED_STATUS = 0x00;

/* Current CPU frequency */
UCHAR cpu_frequency;

/* Functions Declarations */
void mem_allocation_failure_display(void);
void sdk_set_frequency(UCHAR freq);

/* Functions */

/**
 *  \fn       sdk_init_bsp
 *  \breif    This function initializes BSP
 *  \param    None
 *  \return   void
 */
void sdk_init_bsp(void)
{
    /* disable Watch dog timer */
    WDTCTL = WDTPW + WDTHOLD;

    /* do the port initializations */
    halBoardInit();

    /* Initialization routine for XT1 */
    halBoardStartXT1();

    /* Set the inital operating frequency */
    sdk_set_frequency(SYSTEM_CLK);

    /* initialize uart port configuarations */
    sdk_configure_bt_uart();

    /* intiailize hardware units to provide required data(accelerometer and
     * thermometer(optional) */
    sdk_sensor_init();

    /* BT controller realted port configuration */
    sdk_bt_rf_port_config();

    create_user_task();

    /* Configuring MSP430 Timers */
    sdk_config_timer();


    sdk_usb_detected = TRUE;
    /* Initialize the UART1 port which is used as USB serial port */
    halUsbInit(SYSTEM_CLK);
    /* enable the Interrupts */
    __bis_SR_register(GIE);

}

/**
 *  \fn     sdk_init_ui
 *  \par    Description
 *          This function Initializes user interface
 *  \param  None
 *  \return None
 */
void sdk_init_ui(void)
{
    /* intialize the buttons of MSP430 */
    init_buttons();

}

/**
 * \fn      sdk_accelerometer_read
 * \brief   This function is used to read accelerometer data
 * \param   buffer Data buffer
 * \param   buf_len Length of buffer
 * \return  API_SUCCESS/API_FAILURE
 */

API_RESULT sdk_accelerometer_read(UCHAR * buffer, UINT16 buf_len)
{
    // Read the x and y co-ordinate values from accelerometer
    accelerometer_read_cord();
    
    // Adjust raw accelerometer values to scale expected by keyboard events
    // generator app
    // Left/Up ~= 0x3F (63d) --> 0x0A76 (2678d)
    // Center ~= 0x00 (0d) --> 0x0800 (2048d)
    // Right/Down ~= 0xC0 (-64d) --> 0x0580 (1408d)

    volatile int int_accl_x, int_accl_z, int_accl_y;
    int_accl_y = 0;
    int_accl_x = 0;
    int_accl_z = 0;    

    if (accl_z < ACCL_POS_NEG_TH){       // Possitive Acc
    	int_accl_z = accl_z * 10;
    } else{				// Negative Acc
    	int_accl_z = (0xFF - accl_z) * 10;
    	int_accl_z = 0xFFFF - int_accl_z;
    }

    if (accl_y < ACCL_POS_NEG_TH){       // Possitive Acc
    	int_accl_y = accl_y * 10;
    } else{				// Negative Acc
    	int_accl_y = (0xFF - accl_y) * 10;
    	int_accl_y = 0xFFFF - int_accl_y;
    }
    
    if (accl_x < ACCL_POS_NEG_TH){      // Possitive Acc
    	int_accl_x = accl_x * 10;
    } else{				// Negative Acc
    	int_accl_x = (0xFF - accl_x) * 10;
    	int_accl_x = 0xFFFF - int_accl_x;
    }

    int_accl_x += ACCL_SCALE_OFFSET;
    int_accl_y += ACCL_SCALE_OFFSET;
    int_accl_z += ACCL_SCALE_OFFSET;

    // Pack the x and y co-ordinate values in to buffer
    buffer[0] = int_accl_x;
    buffer[1] = (int_accl_x >> 8);
    buffer[2] = int_accl_y;
    buffer[3] = (int_accl_y >> 8);
    buffer[4] = int_accl_z;
    buffer[5] = (int_accl_z >> 8);
    
    return API_SUCCESS;
}


/**
 * \fn      sdk_accelerometer_read
 * \brief   This function is used to read accelerometer data
 * \param   buffer Data buffer
 * \param   buf_len Length of buffer
 * \return  API_SUCCESS/API_FAILURE
 */

API_RESULT sdk_accelerometer_read_int_xyz(int *int_accl_x, int *int_accl_z, int *int_accl_y)
{
    // Read the x and y co-ordinate values from accelerometer
    accelerometer_read_cord();
    
    // Adjust raw accelerometer values to scale expected by keyboard events
    // generator app
    // Left/Up ~= 0x3F (63d) --> 0x0A76 (2678d)
    // Center ~= 0x00 (0d) --> 0x0800 (2048d)
    // Right/Down ~= 0xC0 (-64d) --> 0x0580 (1408d)


    *int_accl_y = 0;
    *int_accl_x = 0;
    *int_accl_z = 0;    

    if (accl_z < ACCL_POS_NEG_TH){       // Possitive Acc
    	*int_accl_z = accl_z * 10;
    } else{				// Negative Acc
    	*int_accl_z = (0xFF - accl_z) * 10;
    	*int_accl_z = 0xFFFF - *int_accl_z;
    }

    if (accl_y < ACCL_POS_NEG_TH){       // Possitive Acc
    	*int_accl_y = accl_y * 10;
    } else{				// Negative Acc
    	*int_accl_y = (0xFF - accl_y) * 10;
    	*int_accl_y = 0xFFFF - *int_accl_y;
    }
    
    if (accl_x < ACCL_POS_NEG_TH){      // Possitive Acc
    	*int_accl_x = accl_x * 10;
    } else{				// Negative Acc
    	*int_accl_x = (0xFF - accl_x) * 10;
    	*int_accl_x = 0xFFFF - *int_accl_x;
    }

    *int_accl_x += ACCL_SCALE_OFFSET;
    *int_accl_y += ACCL_SCALE_OFFSET;
    *int_accl_z += ACCL_SCALE_OFFSET;
    
    return API_SUCCESS;
}

/**
 * \fn      sdk_configure_bt_uart
 * \brief   Function to fill the uart port register and pin settings in the structure
 *          bt_uart_ config
 * \param   None
 * \return  void
 */
void sdk_configure_bt_uart(void)
{
    bt_uart_config.uart_port_sel = &BT_UART_PORT_SEL;
    bt_uart_config.uart_port_dir = &BT_UART_PORT_DIR;
    bt_uart_config.uart_port_out = &BT_UART_PORT_OUT;
    bt_uart_config.uart_tx_pin = BT_UART_TX_PIN;
    bt_uart_config.uart_rx_pin = BT_UART_RX_PIN;
    bt_uart_config.uart_rts_port_dir = &BT_UART_RTS_PORT_DIR;
    bt_uart_config.uart_cts_port_dir = &BT_UART_CTS_PORT_DIR;
    bt_uart_config.uart_rts_pin = BT_UART_RTS_PIN;
    bt_uart_config.uart_cts_pin = BT_UART_CTS_PIN;
    bt_uart_config.uart_cts_port_sel = &BT_UART_CTS_PORT_SEL;
    bt_uart_config.uart_rts_port_sel = &BT_UART_RTS_PORT_SEL;
    bt_uart_config.uart_cts_ies = &BT_UART_CTS_PORT_IES;
    bt_uart_config.uart_cts_ifg = &BT_UART_CTS_PORT_IFG;
    bt_uart_config.uart_cts_ie = &BT_UART_CTS_PORT_IE;
    bt_uart_config.uart_rts_port_out = &BT_UART_RTS_PORT_OUT;
    bt_uart_config.uart_cts_port_out = &BT_UART_CTS_PORT_OUT;
    bt_uart_config.uart_cts_port_in = &BT_UART_CTS_PORT_IN;
    bt_uart_config.uart_cts_port_ren = &BT_UART_CTS_PORT_REN;
    bt_uart_config.uart_reg_ucaxctl0 = &BT_UART_REG_UCAXCTL0;
    bt_uart_config.uart_reg_ucaxctl1 = &BT_UART_REG_UCAXCTL1;
    bt_uart_config.uart_reg_ucaxbr0 = &BT_UART_REG_UCAXBR0;
    bt_uart_config.uart_reg_ucaxbr1 = &BT_UART_REG_UCAXBR1;
    bt_uart_config.uart_reg_ucaxiv = &BT_UART_REG_UCAXIV;
    bt_uart_config.uart_reg_ucaxstat = &BT_UART_REG_UCAXSTAT;
    bt_uart_config.uart_reg_ucaxtxbuf = &BT_UART_REG_UCAXTXBUF;
    bt_uart_config.uart_reg_ucaxrxbuf = &BT_UART_REG_UCAXRXBUF;
    bt_uart_config.uart_reg_ucaxmctl = &BT_UART_REG_UCAXMCTL;
    bt_uart_config.uart_reg_ucaxie = &BT_UART_REG_UCAXIE;
    bt_uart_config.uart_reg_ucaxifg = &BT_UART_REG_UCAXIFG;
    bt_uart_config.uart_baudrate = BAUDRATE_115200;
    bt_uart_config.config_baudrate = BT_UART_CONFIG_BAUDRATE;
}


/**
 * \fn      wrapper_sdk_change_controller_baudrate
 * \brief   Wrapper function to change the uart baudarte of the controller
 * \param   None
 * \return  API_SUCCESS/FAILURE
 */
API_RESULT wrapper_sdk_change_controller_baudrate()
{
    API_RESULT retval;
    retval = sdk_change_controller_baudrate(bt_uart_config.config_baudrate);
    return (retval);
}


/**
 * \fn      sdk_bt_rf_port_config
 * \brief   This function is used to configer pins connecting MSP430 and BT controller
 * \param   None
 * \return  void
 */
void sdk_bt_rf_port_config(void)
{

    /* set ACLK as clock for BT controller */
    BT_RF_CLK_PORT_SEL |= BT_RF_CLK_PORT_PIN;

    /* set BT_RESET as nreset for BT controller */
    BT_RF_NSHUTDOWN_PORT_DIR |= BT_RF_NSHUTDOWN_PORT_PIN;
}

/**
 *  \fn     configTimer1_A3
 *  \par    Description
 *  This    Function configures Timer1_A3, which is used by application as refference timer to handle menu and LED related functionalities.
 *          Timer1_A3 is configured for up mode, thus the timer overflows when
 *          TAR counts to CCR0 and interrupt is generated.
 *          ACLK(32768Hz) is used as clock source and CCR0 is set to 0x8000. So, output Time period is = 1sec.
 *  \param  None
 *  \return None
 */
void configTimer1_A3(void)
{
    TA1CCR0 = 0x8000;
    TA1CCTL0 = CCIE;
    TA1CTL = TASSEL_1 + MC_1 + TACLR;
}


/**
 * \fn      sdk_update_reconnect_error_status
 * \brief   This function is used to clear the error message
 * \param   None
 * \return  void
 */
void sdk_update_reconnect_error_status(void)
{
}

/**
 * \fn      mem_allocation_failure_display
 * \brief   This function is used for error display on memory allocation
 * \param   None
 * \return  void
 */
void mem_allocation_failure_display(void)
{
}

/**
 * \brief 		initializes the accelerometer sensor
 * \param[in]	void
 * \return		none	
 */
void sdk_sensor_init(void)
{
    halI2CInit(SYSTEM_CLK);
    accelerometer_init();

    // Section added for PLT testing - Read Temp sensor value
    halTempStart();
    volatile int testTemp = 0;
    testTemp = halTempRead();
    halTempStop();
}

/**
 * \fn      create_user_task
 * \brief   This function creates the user task
 * \param   None
 * \return  void
 */
void create_user_task(void)
{
    UCHAR ret_val;
    ret_val = BT_thread_create(NULL, NULL, user_task_routine, NULL);
    if (ret_val != 0) {
        sdk_display(SDK_MSG_AREA, (const UCHAR *)
                    "Could not create user Task\n", 0);
    }
}


/**
 * \fn          uart_error_handler
 * \brief 	Function to indicate uart error
 * \param[in]	void
 * \return	none	
 **/
void uart_error_handler(void)
{
    volatile UCHAR uart_rx_val;

    /* Reset sdk status flags */
    sdk_appl_init();
    /* Read the UART RX Buffer value to clear the status register */
    uart_rx_val = *(bt_uart_config.uart_reg_ucaxrxbuf);
    /* Shutdown the controller */
    BT_RF_NSHUTDOWN_PIN_LOW();
    /* Turning OFF LED's to provide visual indication */
    LED_OFF();
}

/**
 * \fn            restore_peripheral_status
 * \brief 	      Function to restore peripheral staus on exiting LPM
 * \param[in]	  void
 * \return	      none	
 **/
void restore_peripheral_status(void)
{
    /* Restore the LED status */
    LED_PORT_OUT |= LED_STATUS;
    /* Initilize the Sensors */
    sdk_sensor_init();
    /* ResumeTimer1_A3 */
    TA1CTL |= MC_1;

    /* Setup MSP430 UART before exiting LPM */
#ifdef SDK_EHCILL_MODE
    MSP430_uart_init();
#endif /* SDK_EHCILL_MODE */
    /* Initialize the UART1 port which is used as USB serial port */
    halUsbInit(SYSTEM_CLK);
}

/**
 * \fn          appl_bt_on_sdk_indication
 * \brief 	Function to indicate bluetooth on is in progress
 * \param[in]	void
 * \return	none	
 **/
void appl_bt_on_sdk_indication(void)
{
    appl_bt_on_in_progress_indication();
}

/**
 * \fn      sdk_set_frequency
 * \brief   Function to set the inital operating frequency
 * \param   freq    Frequency type
 * \return  void
 */
void sdk_set_frequency(UCHAR freq)
{

    halBoardSetSystemClock(freq);
    switch (freq) {
    case SYSCLK_1MHZ:
        cpu_frequency = 1;
        break;
    case SYSCLK_4MHZ:
        cpu_frequency = 4;
        break;
    case SYSCLK_8MHZ:
        cpu_frequency = 8;
        break;
    case SYSCLK_12MHZ:
        cpu_frequency = 12;
        break;
    case SYSCLK_16MHZ:
        cpu_frequency = 16;
        break;
    case SYSCLK_18MHZ:
        cpu_frequency = 18;
        break;
    case SYSCLK_20MHZ:
        cpu_frequency = 20;
        break;
    case SYSCLK_25MHZ:
        cpu_frequency = 25;
        break;
    default:
        break;
    }
}


/**
 * \fn          sdk_error_handler
 * \brief 	Function to indicate error condition
 * \param[in]	void
 * \return	none	
 **/
void sdk_error_handler(const CHAR * error_msg)
{
    BT_RF_NSHUTDOWN_PIN_LOW();
    __disable_interrupt();
    LED_OFF();
    while (1) {
        TOGGLE_LED();
        __delay_cycles(6250000);
    }
}
