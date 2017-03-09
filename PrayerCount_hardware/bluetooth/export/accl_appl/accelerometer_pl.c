
/**
 * Copyright (C) 2009 MindTree Ltd.  All rights reserved.
 *
 * \file    accelerometer_pl.c
 * \brief   This file contains platform specific function definitions for 
 *          accelerometer sensor
 */

/* Header File Inclusion */
#include "sdk_pl.h"
#include "accelerometer_pl.h"
#include "hal_EZ430-RF2560.h"

/* Global variables */
char accl_coord[3];
char accl_x = 0, accl_y = 0, accl_z = 0;

/* Functions */

/**
 * @brief: Function to initialize the accelerometer device
 * @param: None  
 * @return: void
 * @see accelerometer_pl.c    
 */

void accelerometer_init()
{
    halAccStart();
}
/**
 * \brief   read x and y cordinate values of accelerometer output via I2C
 * \param   void
 * \return  void
 */
void accelerometer_read_cord()
{
    /* read the digital accelerometer x- direction and y - direction output */
    halAccRead(accl_coord);
    accl_x = *(accl_coord+0);
    accl_y = *(accl_coord+1);
    accl_z = *(accl_coord+2);
}
