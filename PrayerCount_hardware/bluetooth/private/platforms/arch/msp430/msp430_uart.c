
/**
 * Copyright (C) 2009. MindTree Ltd. All rights reserved.
 *  @file     msp430_uart.c
 *  @brief    This file contains functions related to UART
 */

/* Header File Inclusion */
#include "sdk_pl.h"
#include "BT_queue.h"
#include "task.h"
#include "appl_sdk.h"
#include "msp430_uart.h"
#include "vendor_specific_init.h"

#ifdef MSP430_LPM_ENABLE
extern UCHAR lpm_mode;
#endif /* MSP430_LPM_ENABLE */

extern UCHAR uart_data;
extern DATA_RX_QUEUE data_rx_queue;

extern UCHAR sdk_bt_visible;    /* BT Discoverable On/Off * Status Flag */
extern UCHAR sdk_bt_power;      /* BT Power On/Off Status Flag */
extern CHAR *current_menuText[];

/* Write and Read pointers of circular read buffer */
extern INT16 circular_buf_wt;
extern INT16 circular_buf_rd;
extern volatile UINT16 buf_len;
extern UINT16 bytes_expected;
extern xSemaphoreHandle xReadSemaphore;
extern UCHAR circular_buf[];
/* Current CPU frequency */
extern UCHAR cpu_frequency;

extern void uart_error_handler(void);

/* Global Variables */
volatile UINT32 inactivity_counter = 0;
static UINT32 tx_isr_active;
UART_CONFIG_PARAMS bt_uart_config;

static UCHAR uart_tx_buffer[UART_TX_BUFFER_SIZE];
static UINT16 uart_tx_rd;
static UINT16 uart_tx_wr;


/* Table for uart clock selction based on SYSTEM_CLK */
const UINT32 uart_clk[8] =
    { 1000000, 4000000, 8000000, 12000000, 16000000, 18000000, 20000000,
    25000000
};

const UCHAR UART_params_9600[] = { 0x80, 0x25, 0x00, 0x00 };
const UCHAR UART_params_19200[] = { 0x00, 0x4B, 0x00, 0x00 };
const UCHAR UART_params_38400[] = { 0x00, 0x96, 0x00, 0x00 };
const UCHAR UART_params_57600[] = { 0x00, 0xE1, 0x00, 0x00 };
const UCHAR UART_params_115200[] = { 0x00, 0xC2, 0x01, 0x00 };
const UCHAR UART_params_230400[] = { 0x00, 0x84, 0x03, 0x00 };
const UCHAR UART_params_460800[] = { 0x00, 0x08, 0x07, 0x00 };
const UCHAR UART_params_921600[] = { 0x00, 0x10, 0x0E, 0x00 };


/**
 * \fn      MSP430_uart_init
 * \brief   Initializes UART ports and registers
 * \param   None
 * \return  void
 */
void MSP430_uart_init(void)
{

    /* Set UCSWRST */
    *(bt_uart_config.uart_reg_ucaxctl1) = SWRT;

    /* Initialize USCI registers */
    *(bt_uart_config.uart_reg_ucaxctl0) = UCMODE_0; /* set USCI block to UART */
    *(bt_uart_config.uart_reg_ucaxctl0) &= ~UC7BIT; /* select 8bit char mode */

    if (SDK_IS_BT_POWERED_ON()) {
        uart_set_baudrate(bt_uart_config.config_baudrate);
    } else {
        uart_set_baudrate(bt_uart_config.uart_baudrate);
    }

    /* Configure ports */
    *(bt_uart_config.uart_port_sel) |=
        bt_uart_config.uart_tx_pin + bt_uart_config.uart_rx_pin;
    *(bt_uart_config.uart_port_dir) |= bt_uart_config.uart_tx_pin;
    *(bt_uart_config.uart_port_dir) &= ~bt_uart_config.uart_rx_pin;

    /* CTS AND RTS DIRECTION SETTINGS */
    *(bt_uart_config.uart_cts_port_dir) &= ~(bt_uart_config.uart_cts_pin);
    *(bt_uart_config.uart_cts_port_ren) |= (bt_uart_config.uart_cts_pin);
    *(bt_uart_config.uart_cts_port_out) |= (bt_uart_config.uart_cts_pin);

    *(bt_uart_config.uart_rts_port_dir) |= (bt_uart_config.uart_rts_pin);

    /* Initializing PxSEL for CTS\RTS pins */
    *(bt_uart_config.uart_cts_port_sel) &= ~(bt_uart_config.uart_cts_pin);
    *(bt_uart_config.uart_rts_port_sel) &= ~(bt_uart_config.uart_rts_pin);

    /* clear UCSWRST */
    *(bt_uart_config.uart_reg_ucaxctl1) &= ~SWRT;

    /* ENABLE uart RX interrupt */
    UART_ENABLE_BT_UART_RX();
}


/**
 * \fn      BT_UART_ISR
 * \brief   This function is an ISR for BT uart
 * \param None
 * \return void
 */
#ifndef TOOLCHAIN_IAR
#pragma CODE_SECTION(BT_UART_ISR, ".ISR");
#endif /* TOOLCHAIN_IAR */

#pragma vector=BT_UART_VECTOR
__interrupt void BT_UART_ISR(void)
{
    UINT16 int_vect;
    INT16 bytes_rxed_in_read_buf = 0;
    signed portBASE_TYPE xHigherPriorityTaskWoken;

    UART_DISABLE_BT_UART_RTS();

    inactivity_counter = 0;
    xHigherPriorityTaskWoken = pdFALSE;

    int_vect = *(bt_uart_config.uart_reg_ucaxiv);
    /* RX interrupt handler */
    if (int_vect & 0x02) {
        volatile UCHAR rx_octet;

        if (((*(bt_uart_config.uart_reg_ucaxstat)) & UCRXERR)) {
            /* Handle UART error */
            uart_error_handler();
        } else {
            rx_octet = *(bt_uart_config.uart_reg_ucaxrxbuf);
            UPDATE_RX_BUFFER(rx_octet);
            bytes_rxed_in_read_buf = circular_buf_wt - circular_buf_rd;
            /* To handle roll over condition of read buffer */
            if (bytes_rxed_in_read_buf < 0) {
                bytes_rxed_in_read_buf =
                    (MAX_CIRCULAR_BUF_SIZE - circular_buf_rd) + circular_buf_wt;
            }

            /* Unlocking read task after receiving required number of bytes */
            if (bytes_rxed_in_read_buf >= bytes_expected) {
                buf_len = bytes_expected;
                UART_DISABLE_BT_UART_RX();
                xSemaphoreGiveFromISR(xReadSemaphore,
                                      &xHigherPriorityTaskWoken);
            }
        }
    }

    /* TX INTERRUPT HANDLER */
    if (int_vect & 0x04) {
        UINT16 saved_wr, avail;

        Q_BUFFER_UPDATE_RDWR_PTR(UART_TX_BUFFER_SIZE, uart_tx_rd, 1);
        saved_wr = uart_tx_wr;

        /* Check if there are enough space to write the require number of bytes 
         */
        Q_BUFFER_GET_COUNT(UART_TX_BUFFER_SIZE, uart_tx_rd, saved_wr, avail);

        if (avail == 0) {
            tx_isr_active = 0;
        } else {
            tx_isr_active = 1;
            UART_TRANSMIT(uart_tx_buffer[uart_tx_rd]);
        }
    }
#ifdef MSP430_LPM_ENABLE
    if (TRUE == lpm_mode) {
        lpm_mode = FALSE;
        restore_peripheral_status();
        /* Exit LPM */
        EXIT_MSP430_LPM();
    }
#endif /* MSP430_LPM_ENABLE */

    /* Force a context switch if xHigherPriorityTaskWoken was set to true */
    if (xHigherPriorityTaskWoken) {
        portYIELD();
    }
}


/**
 * \fn      hci_uart_bt_shutdown
 * \brief   This function is used for HCI-UART Bluetooth-OFF Shutdown
 * \param   void
 * \return  void
 */
void hci_uart_bt_shutdown(void)
{
    /* Disable UART interrupt */
    *(bt_uart_config.uart_reg_ucaxie) &= ~UCRXIE;
    /* Reset UART state */
    *(bt_uart_config.uart_reg_ucaxctl1) = UCSWRST;
    *(bt_uart_config.uart_port_out) |=
        (bt_uart_config.uart_tx_pin + bt_uart_config.uart_rx_pin);
    /* Configure UART TX and Rx as pins as GPIO */
    *(bt_uart_config.uart_port_sel) &=
        ~(bt_uart_config.uart_tx_pin + bt_uart_config.uart_rx_pin);
    /* Configure TX and Rx GPIO pins as Output pins */
    *(bt_uart_config.uart_port_dir) |= bt_uart_config.uart_tx_pin;
    *(bt_uart_config.uart_port_dir) |= bt_uart_config.uart_rx_pin;
}


/**
 * \fn      hci_uart_write_data
 * \brief   This function writes data to the hci uart
 * \param   buf     Data buffer
 * \param   length  Length of data buffer
 * \return  void
 */
void hci_uart_write_data(UCHAR * buf, UINT16 length)
{
    UINT16 saved_rd, space, linear_space;

    saved_rd = uart_tx_rd;

    /* Check if there are enough space to write the require number of bytes */
    Q_BUFFER_GET_SPACE(UART_TX_BUFFER_SIZE, saved_rd, uart_tx_wr, space);

    if (space < length) {
        return;
    }

    /* If yes, write the bytes in the tx circular buffer */
    Q_BUFFER_GET_SPACE_LINEAR(UART_TX_BUFFER_SIZE, saved_rd, uart_tx_wr,
                              linear_space);

    /* Increament counter */
    if (linear_space >= length) {
        BT_mem_copy(&uart_tx_buffer[uart_tx_wr], buf, length);
        Q_BUFFER_UPDATE_RDWR_PTR(UART_TX_BUFFER_SIZE, uart_tx_wr, length);
    } else {
        BT_mem_copy(&uart_tx_buffer[uart_tx_wr], buf, linear_space);
        Q_BUFFER_UPDATE_RDWR_PTR(UART_TX_BUFFER_SIZE, uart_tx_wr, linear_space);

        buf += linear_space;
        linear_space = length - linear_space;

        BT_mem_copy(&uart_tx_buffer[uart_tx_wr], buf, linear_space);
        Q_BUFFER_UPDATE_RDWR_PTR(UART_TX_BUFFER_SIZE, uart_tx_wr, linear_space);
    }

    if (0 == tx_isr_active) {
        UART_TRANSMIT(uart_tx_buffer[uart_tx_rd]);
    }

    /* Enable Tx interrupt */
    UART_ENABLE_BT_UART_TX();
}


/**
 * \fn      uart_setup
 * \brief   This function is used to initialize uart
 * \param   None
 * \return  void
 */
void uart_setup()
{
    /* Initialize the circular buffer realted pointers and counts */
    circular_buf_wt = 0;
    circular_buf_rd = 0;

    memset(&data_rx_queue, 0x00, sizeof(DATA_RX_QUEUE));

    uart_data = 0;
    tx_isr_active = 0;
    /* Initialize UART ports and registers */
    MSP430_uart_init();

}

/**
 * \fn      uart_set_baudrate
 * \brief   This function is used to set the uart baudrate
 * \param   Baudarate    the value of baudrate to be set
 * \return  void
 */
void uart_set_baudrate(UINT32 baudrate)
{
    /* SMCLK is set as UART clock source */
    *(bt_uart_config.uart_reg_ucaxctl1) |= UCSSEL_2;

    switch (baudrate) {
    case BAUDRATE_115200:
        switch (cpu_frequency) {
        case 8:
            /* Using UCOS16 = 0 settings */
            *(bt_uart_config.uart_reg_ucaxbr1) = 0x00;
            *(bt_uart_config.uart_reg_ucaxbr0) = 0x45;
            *(bt_uart_config.uart_reg_ucaxmctl) = 0x08;
            break;
        case 12:
            /* Using UCOS16 = 0 settings */
            *(bt_uart_config.uart_reg_ucaxbr1) = 0x00;
            *(bt_uart_config.uart_reg_ucaxbr0) = 0x68;
            *(bt_uart_config.uart_reg_ucaxmctl) = 0x02;
            break;
        case 18:
            /* Using UCOS16 = 0 settings */
            *(bt_uart_config.uart_reg_ucaxbr1) = 0x00;
            *(bt_uart_config.uart_reg_ucaxbr0) = 0x9C;
            *(bt_uart_config.uart_reg_ucaxmctl) = 0x04;
            break;
        case 25:
            /* Using UCOS16 = 0 settings */
            *(bt_uart_config.uart_reg_ucaxbr1) = 0x00;
            *(bt_uart_config.uart_reg_ucaxbr0) = 0xD9;
            *(bt_uart_config.uart_reg_ucaxmctl) = 0x00;
            break;
        default:
            break;
        }
        break;
    case BAUDRATE_921600:
        switch (cpu_frequency) {
        case 25:
            /* Using UCOS16 = 0 settings */
            *(bt_uart_config.uart_reg_ucaxbr1) = 0x00;
            *(bt_uart_config.uart_reg_ucaxbr0) = 0x1B;
            *(bt_uart_config.uart_reg_ucaxmctl) = 0x02;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

/**
 * \fn      sdk_change_controller_baudrate
 * \brief   Function to change the uart baudarte of the controller
 * \param   baudarate   Baud rate to be set
 * \return  API_SUCCESS/FAILURE
 */
API_RESULT sdk_change_controller_baudrate(UINT32 baudarate)
{
    UCHAR *bt_rf_uart_update_params;
    API_RESULT retval;

    switch (baudarate) {
    case BAUDRATE_9600:
        bt_rf_uart_update_params = (UCHAR *) & UART_params_9600[0];
        break;
    case BAUDRATE_19200:
        bt_rf_uart_update_params = (UCHAR *) & UART_params_19200[0];
        break;

    case BAUDRATE_38400:
        bt_rf_uart_update_params = (UCHAR *) & UART_params_38400[0];
        break;

    case BAUDRATE_57600:
        bt_rf_uart_update_params = (UCHAR *) & UART_params_57600[0];
        break;

    case BAUDRATE_115200:
        bt_rf_uart_update_params = (UCHAR *) & UART_params_115200[0];
        break;

    case BAUDRATE_230400:
        bt_rf_uart_update_params = (UCHAR *) & UART_params_230400[0];
        break;
    case BAUDRATE_460800:
        bt_rf_uart_update_params = (UCHAR *) & UART_params_460800[0];
        break;
    case BAUDRATE_921600:
        bt_rf_uart_update_params = (UCHAR *) & UART_params_921600[0];
        break;
    default:
        bt_rf_uart_update_params = (UCHAR *) & UART_params_115200[0];
        break;
    }

    /* send the vendor specifc command to BT controller */
    retval =
        BT_hci_vendor_specific_command(SDK_BT_RF_UPDATE_UART_BAUDRATE_OPCODE,
                                       (UCHAR *) bt_rf_uart_update_params,
                                       SDK_BT_RF_UPDATE_UART_BAUDRATE_COMMAND_LEN);

    if (API_SUCCESS != retval) {
        printf("FAILED to send vendor specific command. retval = %02X\n",
               retval);
    }

    return (retval);
}

/**
 * \fn      sdk_change_host_baudrate
 * \brief   Function to change the UART baudrate of the hos
 * \param   baudarate   Baud rate to be set
 * \return
 */
API_RESULT sdk_change_host_baudrate(UINT32 baudarate)
{
    uart_set_baudrate(baudarate);
    return API_SUCCESS;
}
