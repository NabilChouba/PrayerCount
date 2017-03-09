
/**
 * Copyright (C) 2000 MindTree Ltd. All rights reserved.
 * \file    hci_uart.h
 * \brief   Contains the definitions used by the HCI UART Transport Layer
 */

#ifndef _H_HCI_UART_
#define _H_HCI_UART_

/** --------------------------------- Header File Inclusion */

#include "BT_common.h"


/* ---------------------------------- macro defintitions */



/* ---------------------------------- Global Definitions */
#if defined BT_UART && !defined BT_BCSP
#define hci_transport_write_data    hci_uart_send_data
#endif /* BT_UART && !BT_BCSP */

#ifdef BT_BCSP
#define hci_transport_write_data    hci_bcsp_send_data
#endif /* BT_BCSP */


/* ---------------------------------- Function Declarations */

/** HCI UART Initialization & Shutdown */
void hci_uart_init(void);
void hci_uart_bt_init(void);
void hci_uart_bt_shutdown(void);

/* HCI UART Send Data */
API_RESULT hci_uart_send_data(UCHAR, UCHAR *, UINT16, UCHAR);

#endif /* _H_HCI_UART_ */
