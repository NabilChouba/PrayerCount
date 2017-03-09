
/**
 *  @file measurement.h
 *
 *  This file contains macros/datatype specific to CPU utilization calculation
 */

/* 
 *  Copyright (C) 2009-2010. MindTree Ltd.
 *  All rights reserved.
 */

#ifndef _H_MEASUREMENT_
#define _H_MEASUREMENT_

typedef struct cpuUtilMeasurement {
    volatile UINT32 taskStart;
    volatile UINT32 taskEnd;
    volatile UINT32 taskDuration;
    UINT8 taskStartFlag;
} cpuUtilMeasurement_t;


#define START_TASK_CALCULATION()\
        if (0 == dataStats.taskStartFlag)                       \
        {                                                       \
            dataStats.taskStart = resetTimerBCounter();         \
            dataStats.taskEnd = 0;                              \
            dataStats.taskStartFlag = 1;                        \
        }                                                       \


#define END_TASK_CALCULATION()\
        if (dataStats.taskStartFlag)                                        \
        {                                                                   \
	  dataStats.taskEnd = readTimerBCounter();                          \
          dataStats.taskDuration = (dataStats.taskEnd - dataStats.taskStart);\
	  totalTimerCount_g += dataStats.taskDuration;                       \
          dataStats.taskStartFlag = 0;                                       \
        }                                                                    \



void configTimerB(void);
void configRTC(void);
UINT16 resetTimerBCounter(void);
UINT16 readTimerBCounter(void);
void resetMeasurementParams(void);
void configMSP430Timers(void);
void LcdDisplay(void);
void LCD_disp_utilz(UINT32 util_count);

#endif /* _H_MEASUREMENT */
