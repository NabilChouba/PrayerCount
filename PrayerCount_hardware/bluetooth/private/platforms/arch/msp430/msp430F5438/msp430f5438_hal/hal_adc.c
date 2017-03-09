/**
 * Copyright (C) 2010 Texas Instruments, Inc.
 *
 * \file    hal_adc.c
 * \brief   This file contains functions to handle internal MSP430 ADC
 * \version July 2010
 */

#include  <msp430x54xA.h>
#include "hal_MSP-EXP430F5438.h"
#include "hci_uart.h"
#include "sdk_pl.h"

extern UART_CONFIG_PARAMS bt_uart_config;

int SavedADC12MEM0 = 0, SavedADC12MEM1 = 0, SavedADC12MEM2 = 0;
int SavedADC12MEM3 = 0;
long int Vcc = 0, vcc_value = 0;

float V30 = 0, V85 = 0;
float mref15 = 0, nref15 = 0;
float mref25 = 0, nref25 = 0;
volatile struct s_TLV_ADC_Cal_Data *pADCCAL;

unsigned char conversionType = FAHRENHEIT, adcMode = ADC_OFF_MODE;
unsigned char exit_active_from_ADC12 = 0;

/**
 * @brief  Intializes the ADC12 to sample Temperature and Vcc.
 * @param  none
 * @return none
 */
void halAdcInitTempVcc(void)
{
    /* Sequence of channels, once, */
    adcMode = ADC_TEMP_MODE;
    UCSCTL8 |= MODOSCREQEN;
    ADC12CTL0 = ADC12ON + ADC12SHT0_15 + ADC12MSC + +ADC12REFON + ADC12REF2_5V;
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1 + ADC12SSEL_0;
    ADC12CTL2 = ADC12RES_2;

    ADC12MCTL0 = ADC12SREF_1 + TEMP_CHANNEL;
    ADC12MCTL1 = ADC12SREF_1 + VCC_CHANNEL + ADC12EOS;
}


/**
 * @brief  Turns off / disable the ADC12.
 * @param  none
 * @return none
 */
void halAdcShutDownTempVcc(void)
{
    ADC12CTL0 &= ~(ADC12ON + ADC12ENC + ADC12REFON);
    adcMode = ADC_OFF_MODE;
}

/**
 * @brief  Sets the conversion type to either Farenheit (F) or Celsius (C).
 * @param  conversion The #define constant CELSIUS or FAHRENHEIT.
 * @return none
 */
void halAdcSetTempConversionType(unsigned char conversion)
{
    conversionType = conversion;
}


/**
 * @brief  Get function for the current Vcc value.
 * @param  none
 * @return The current Vcc value.
 */
int halAdcGetVcc(void)
{
    return Vcc;
}

/**
 * @brief  Converts the Vcc and Temp readings from the ADC to BCD format.
 * @param  none
 * @return none
 */
long int halAdcConvertTempVccFromADC(void)
{
    float Temperature = 0;
    // Convert Vcc
    Vcc = SavedADC12MEM1;
    Vcc = Vcc * 50 + 2048;      // add .5*4096 so truncation rounds up
    Vcc = Vcc / 4096;
    vcc_value = Vcc;
    /* The temperature value is multiplied by 10 so that to be displyed in XX.X
     * format */
    Temperature = ((float)SavedADC12MEM0 * 5 * 10) / 8192;
    Temperature = (Temperature - (nref25 * 10)) / (mref25);

    if (conversionType == FAHRENHEIT) {
        Temperature = Temperature * 9 / 5 + (32 * 10);
    }
    return (long int)Temperature;
}

/**
 * @brief  Get function for the temperature and Vcc samples in "xxx^C/F" and
 *         "x.xV" format.
 * @param  TemperatureStr The string that holds the temperature reading
 * @param  Vcc            The string that holds the Vcc reading
 * @return none
 */
void haldisptemp(char *TemperatureStr, long int temp_val)
{
    unsigned char i, leadingZero = 0;
    long int Temperature;

    Temperature = temp_val;
    for (i = 0; i < 8; i++)
        TemperatureStr[i] = '\0';
    i = 0;
    /* Check for negative */
    if (Temperature < 0) {
        TemperatureStr[i++] = '-';
        Temperature = -Temperature;
    }
    TemperatureStr[i] = '0';
    if (Temperature >= 1000) {
        TemperatureStr[i] = '1';
        Temperature -= 1000;
        leadingZero = 1;
    }
    if (leadingZero == 1)
        i++;
    /* 100s digit */
    TemperatureStr[i] = '0';
    if (Temperature >= 100) {
        do {
            TemperatureStr[i]++;
            Temperature -= 100;
        }
        while (Temperature >= 100);
        leadingZero = 1;
    }
    if (leadingZero == 1)
        i++;
    /* 10s digit */
    TemperatureStr[i] = '0';
    if (Temperature >= 10) {
        do {
            TemperatureStr[i]++;
            Temperature -= 10;
        }
        while (Temperature >= 10);
    }
    TemperatureStr[++i] = '.';
    TemperatureStr[++i] = Temperature + '0';

    TemperatureStr[++i] = '^';
    if (conversionType == CELSIUS)
        TemperatureStr[++i] = 'C';
    else
        TemperatureStr[++i] = 'F';
}

/*----------------------------------------------------------------------------*/

/**
 * @brief  Starts the ADC conversion.
 * @param  none
 * @return none
 */
void halAdcStartRead(void)
{
    ADC12IFG &= ~(BIT2 + BIT1 + BIT0);  /* Clear any pending flags */

    if (adcMode == ADC_ACC_MODE) {
        ADC12CTL0 |= ADC12ENC | ADC12SC;
        ADC12IE |= BIT3;
    } else {
        ADC12CTL0 |= ADC12REFON;    /* Turn on ADC12 reference */

        /* Delay to stabilize ADC12 reference assuming the fastest MCLK of 18 */
        /* MHz.  */
        /* 35 us = 1 / 18 MHz * 630 */
        /* Commenting the delay for thermometer application */

    /*__delay_cycles(630); */

        ADC12IE |= BIT1;        /* Enable interrupt */
        ADC12CTL0 |= ADC12ENC | ADC12SC;
    }
}

/**
 * @brief  Sets the flag that causes an exit into active CPU mode from
 *         the ADC12 ISR.
 * @param  quit
 * - 1 - Exit active from ADC12 ISR
 * - 0 - Remain in LPMx on exit from ADC12ISR
 * @return none
 */
void halAdcSetQuitFromISR(unsigned char quit)
{
    exit_active_from_ADC12 = quit;
}

/**
 * @brief  Calibrate the ADC
 * @param  none
 * @return none
 */
void halBoardInitADCCalibration(void)
{
    unsigned char bADCCAL_bytes;

    /* Check for ADC TLV using the tag 0x10 for F5438 and 0x11 for F5438A */
    if (Get_Device_Type() == F5438)
        Get_TLV_info(0x10, &bADCCAL_bytes, (unsigned int **)&pADCCAL);
    else
        Get_TLV_info(0x11, &bADCCAL_bytes, (unsigned int **)&pADCCAL);

    /* *** Calculation of the temp slope & offset** */
    /* Reference 1,5V */
    V30 = ((float)pADCCAL->adc_ref15_30_temp * 1.5) / 4096;
    V85 = ((float)pADCCAL->adc_ref15_85_temp * 1.5) / 4096;
    mref15 = (V85 - V30) / (85 - 30);
    nref15 = V85 - mref15 * 85;

    /**
	 * For release F5438A revisions, comment out the if/else and leave the V30,
	 * V85 calculation below. The offset from the datasheet is +- 20 deg C, so a
     * wide variance in the temperature calculations for the F5438A is to be
     * expected if the .00225 slope and 680 mV offset are used.
	 */
    if (Get_Device_Type() == F5438) {
        // Reference 2,5V
        V30 = ((float)pADCCAL->adc_ref25_30_temp * 2.5) / 4096;
        V85 = ((float)pADCCAL->adc_ref25_85_temp * 2.5) / 4096;
        mref25 = (V85 - V30) / (85 - 30);
        nref25 = V85 - mref25 * 85;
    } else {
        mref25 = .00225;        /* .00225 slope per the datasheet */
        nref25 = .680;          /* 680 mV offset per the datasheet */
    }
}

/*----------------------------------------------------------------------------*/
#ifndef TOOLCHAIN_IAR
#pragma CODE_SECTION(ADC12_ISR,".ISR");
#endif /* TOOLCHAIN_IAR */
#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    UART_DISABLE_BT_UART_RTS();
    ADC12IFG = 0;               /* Clear the interrupt flags */
    ADC12CTL0 &= ~(ADC12ENC | ADC12SC | ADC12REFON);

    __bis_SR_register(GIE);

    SavedADC12MEM0 = ADC12MEM0; /* Store the sampled data */
    SavedADC12MEM1 = ADC12MEM1;
    SavedADC12MEM2 = ADC12MEM2; /* Store the sampled data */
    SavedADC12MEM3 = ADC12MEM3;

}
