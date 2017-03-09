
/**
 *  @file arch/msp430/sdk_pl.h
 *
 *  @brief This file contains function/datatype specific to this platform
 */

/* 
 *  Copyright (C) 2009-2010. MindTree Ltd.
 *  All rights reserved.
 */

#ifndef _H_SDK_PL_
#define _H_SDK_PL_

/* ----------------------------------------------- Header File Inclusion */
#include "appl_menu_pl.h"
#include "sdk_config.h"
#include "accelerometer_pl.h"
#include "msp430_uart.h"
#include "hci_uart.h"
#include "BT_hci_api.h"
#include "task.h"
/* ----------------------------------------------- Macros */

/* Display Area Type */
#define SDK_MENU_AREA   0x01
#define SDK_STATUS_AREA 0x02
#define SDK_MSG_AREA    0x03

/* Bluetooth On status */
#define SDK_BT_OFF                    0x01
#define SDK_BT_ON_IN_PROGRESS         0x02
#define SDK_BT_ON                     0x03

/* Bluetooth Visibility status */
#define SDK_DISC_OFF    0x01
#define SDK_DISC_ON     0x02

/* Bluetooth Play On/Off status */
#define SDK_SPP_TX_OFF    0x01
#define SDK_SPP_TX_ON     0x02

#define SDK_ACTIVE      0x01
#define SDK_IN_SNIFF    0x02
#define SDK_OFF         0x03
#define SDK_IS_BT_POWERED_ON()        (SDK_BT_ON == sdk_bt_power)
#define SDK_IS_BT_POWERED_OFF()       (SDK_BT_OFF == sdk_bt_power)
#define SDK_IS_BT_DISCOVERABLE()      (SDK_DISC_ON == sdk_bt_visible)
#define SDK_IS_BT_NON_DISCOVERABLE()  (SDK_DISC_OFF == sdk_bt_visible)
#define SDK_IS_SPP_TX_STARTED(index)    (SDK_SPP_TX_ON == sdk_status[(index)].sdk_data_sending)


/* 
 * mapping to the defined for user in sdk_config.h to the
 * defintions in msp430x54x.h
 */
#define MSP430_LPM_0 LPM0_bits
#define MSP430_LPM_1 LPM1_bits
#define MSP430_LPM_2 LPM2_bits
#define MSP430_LPM_3 LPM3_bits
#define MSP430_LPM_4 LPM4_bits


/* LPM related states */
#define ENTER_LPM_OFF	0x00
#define ENTER_LPM_ON	0x01

/* macro to enter MSP430 low power mode */
#define ENTER_MSP430_LPM()	__bis_SR_register(SDK_MSP430_LPM)
/* macro to exit MSP430 low power mode */
#define EXIT_MSP430_LPM()	__bic_SR_register_on_exit(SDK_MSP430_LPM)

#define TOGGLE_LED1()	LED_PORT_OUT ^= LED_1;
#define TOGGLE_LED2()	LED_PORT_OUT ^= LED_2;
#define TOGGLE_LED()	LED_PORT_OUT ^= (LED_1 | LED_2);

#define LED1_ON()		LED_PORT_OUT |= LED_1;
#define LED2_ON()		LED_PORT_OUT |= LED_2;
#define LED_ON()		LED_PORT_OUT |= (LED_1 | LED_2);

#define LED1_OFF()		LED_PORT_OUT &= ~LED_1;
#define LED2_OFF()		LED_PORT_OUT &= ~LED_2;
#define LED_OFF()		LED_PORT_OUT &= ~(LED_1 | LED_2);

#define appl_bluetooth_on_indication()		LED1_ON()
#define appl_bluetooth_off_indication()		LED_OFF()
#define appl_bt_on_in_progress_indication()	TOGGLE_LED1()
#define appl_spp_connect_indication()		LED2_ON()
#define appl_spp_data_send_indication()		TOGGLE_LED2()
#define appl_spp_data_recev_indication()	TOGGLE_LED2()

#define READ_TASK_QUEUE_LEN		10
#define WRITE_TASK_QUEUE_LEN	        11
#define MENU_TASK_QUEUE_LEN		12


/* BT controller vendor specific command opcode to update the uart baudrate */
#define SDK_BT_RF_UPDATE_UART_BAUDRATE_OPCODE        0xFF36
#define SDK_BT_RF_UPDATE_UART_BAUDRATE_COMMAND_LEN   0x04
/* BT controller vendor specific command opcode to set the controller BD
 * Address */
#define SDK_BT_RF_SET_BD_ADDR_OPCODE                 0xFC06
#define SDK_BT_RF_SET_BD_ADDR_COMMAND_LEN	           0x06
#define SDK_BT_RF_SET_LPS_PARAMS                     0xFD2E

#define SDK_BT_OFF_PROCESSED	0x00
#define SDK_BT_OFF_REQUESTED 	0x01
/* define the timeout for LCD contrast adjusting */
#define SET_CONTRAST_TIMEOUT 60

/* Set MAX Power Opcodes */
#define SDK_BT_RF_SET_POWER_VECTOR            0xFD82
#define SDK_BT_RF_SET_CLASS_2_SINGLE_POWER    0xFD87
/* Enable Calibration Opcodes */
#define SDK_BT_RF_SET_CALIBRATION             0xFD76
#define SDK_BT_RF_ENABLE_CALIBRATION          0xFD80

#define sdk_start_scheduler() vTaskStartScheduler()
#define UPDATE_RX_BUFFER(data) \
      { \
        circular_buf[circular_buf_wt] = data;\
        circular_buf_wt++;\
        if (circular_buf_wt == MAX_CIRCULAR_BUF_SIZE) {\
                circular_buf_wt = 0;\
        }\
      }

#define UPDATE_USER_BUFFER(data) \
      { \
        circular_user_buffer[circular_user_buf_wt] = data;\
        circular_user_buf_wt++;\
        if (circular_user_buf_wt == MAX_USER_BUF) {\
                circular_user_buf_wt = 0;\
        }\
      }

/* ----------------------------------------------- Functions */
void sdk_init_bsp(void);

#define sdk_display(...)

#ifndef SDK_SKIP_UI_INIT
void sdk_init_ui(void);
#endif /* SDK_SKIP_UI_INIT */

/* Function to read data from accelerometer */
API_RESULT sdk_accelerometer_read(UCHAR * buffer, UINT16 buf_len);
API_RESULT sdk_accelerometer_read_int_xyz(int *int_accl_x, int *int_accl_z, int *int_accl_y);

API_RESULT sdk_change_controller_baudrate(UINT32 baudarate);
API_RESULT sdk_change_host_baudrate(UINT32 baudarate);

API_RESULT sdk_accelerometer_init(void);

void sdk_configure_bt_uart(void);
void sdk_bt_rf_port_config(void);

/* Macro to set the CC2560 nSHUTDOWN pin high */
#define BT_RF_NSHUTDOWN_PIN_HIGH()  \
BT_RF_NSHUTDOWN_PORT_OUT |= BT_RF_NSHUTDOWN_PORT_PIN;

/* Macro to set the CC2560 nSHUTDOWN pin low */
#define BT_RF_NSHUTDOWN_PIN_LOW()  \
BT_RF_NSHUTDOWN_PORT_OUT &= ~BT_RF_NSHUTDOWN_PORT_PIN;


/* Macros to enter and exit flight mode */
#define SDK_ENTER_FLIGHT_MODE	BT_RF_NSHUTDOWN_PIN_LOW
#define SDK_EXIT_FLIGHT_MODE	BT_RF_NSHUTDOWN_PIN_HIGH
/* Function to set the local device BD Address */
void sdk_set_bd_addr(void);

/* Function to display main menu */
void appl_sdk_display_status(UCHAR staus_update_option);
void sdk_sensor_init(void);
void configTimer1_A3(void);

/* Initialize UART ports and registers */
void MSP430_uart_init(void);

/* define printf to NULL for MSP430 */
#define printf(...)

/* function clear the error message */
void sdk_update_reconnect_error_status(void);

void uart_enable_cts_interrupt(void);

void uart_disable_cts_interrupt(void);

API_RESULT wrapper_sdk_change_controller_baudrate();

void hci_uart_write_data(UCHAR *, UINT16);

void create_user_task(void);

void *user_task_routine(void *args);

/* Configuring MSP430 Timers */
void sdk_config_timer(void);

/* This function does the uart initilizations of MSP430 */
void uart_setup(void);
void uart_set_baudrate(UINT32 baudrate);
/* Function to restore peripheral staus on exiting LPM */
void restore_peripheral_status(void);

/**
 * @brief   To enable max-power setting by user application
 * @param   max_power in dBm (OP_POWER_6 or OP_POWER_10)
 * @return  API_SUCCESS/API_FAILURE
 *
 * @see     vendor_specific_init.c
 */

API_RESULT sdk_set_max_output_power(OUTPUT_POWER_LEVEL max_power);

void sdk_error_handler(const CHAR * error_msg);

#endif /* _H_SDK_PL_ */
