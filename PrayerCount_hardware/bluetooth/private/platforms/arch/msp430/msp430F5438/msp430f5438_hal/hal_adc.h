/**
 * Copyright (C) 2010 Texas Instruments, Inc.
 *
 * \file    hal_adc.h
 * \brief   hal_adc.c header file
 * \version July 2010
 */

#ifndef HAL_ADC_H
#define HAL_ADC_H

#define TEMP_CHANNEL      ADC12INCH_10
#define VCC_CHANNEL       ADC12INCH_11

#define CELSIUS 			0xFF
#define FAHRENHEIT 			0x00

#define CELSIUS_MUL_F5438	 	7040
#define CELSIUS_OFFSET_F5438 		2620
#define FAHRENHEIT_MUL_F5438 		12672
#define FAHRENHEIT_OFFSET_F5438	 	3780

#define CELSIUS_MUL_F5438A	 	7040
#define CELSIUS_OFFSET_F5438A 		2620
#define FAHRENHEIT_MUL_F5438A 		12672
#define FAHRENHEIT_OFFSET_F5438A	3780

/*-------------Temperature & VCC Functions------------------------------------*/
void halAdcInitTempVcc(void);
void halAdcShutDownTempVcc(void);
void halAdcSetTempConversionType(unsigned char conversion);
int halAdcGetTemp(void);
int halAdcGetVcc(void);
long int halAdcConvertTempVccFromADC(void);
void halAdcReadTempVcc(char *TemperatureStr, char *VccStr);

/*-------------Generic ADC12 Functions----------------------------------------*/
void halAdcStartRead(void);
void halAdcSetQuitFromISR(unsigned char quit);
void halBoardInitADCCalibration(void);
void haldisptemp(char *TemperatureStr, long int temp_val);

#endif
