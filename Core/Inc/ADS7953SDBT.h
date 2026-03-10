/*
 * ADS7953SDBT.h
 *
 * Created on: Mar 10, 2026
 * Author: Subrat
 */

#ifndef INC_ADS7953SDBT_H_
#define INC_ADS7953SDBT_H_

#include "main.h"

extern SPI_HandleTypeDef hspi5;

/*commands to set the modes
 * of the ADC*/
#define         MANUAL_MODE             0b0001101011000000
#define         AUTO_1_MODE             0b0010110000001111
#define         AUTO_2_MODE             0b0011110000000000
#define         AUTO_2_MODE_NO_RST      0b0011100000000000

/*commands to program the modes of the ADC*/
#define         AUTO_1_PROGRAM          0b1000000000000000
#define         AUTO_1_SEQUENCE         0b0111111111111111
#define         AUTO_2_PROGRAM_ADC1     0b1001001111000000
/* #define      AUTO_2_PROGRAM_ADC2     0b1001001011000000 */

/*command to keep operating in the same selected mode*/
#define         CONTINUE_OPERATION      0b0000000000000000

/*commands for GPIO program register settings*/
#define         GPIO_PROGRAM            0b0100001

typedef enum _operation_modes{
    SELECT_ADC = 1,
    READ_ADC = 2
} operation_modes;

void ADC1_Enable(void);
void ADC1_Disable(void);

/* Commented out ADC2 */
// void ADC2_Enable();
// void ADC2_Disable();

void ADC_Operate(uint16_t *adc1_channels);

void AUTO_2_Select_ADC1(operation_modes mode);
void AUTO_2_Program_ADC1(void);

/* Commented out ADC2 AUTO_2 functions */
// void AUTO_2_Select_ADC2(operation_modes mode);
// void AUTO_2_Program_ADC2();

void Continue_Operaion(uint8_t *data);
void ADC_CombineData(uint16_t *adc1_channels);

void ADC1_RawConv_Data(float *adc1_conv_buf, uint16_t *adc1_buf);
/* void ADC2_RawConv_Data(float *adc2_conv_buf, uint16_t *adc2_buf); */

void ADC1_Volt_Temp_Conv(float *adc1_conv_buf, float *temp_buf);

#endif /* INC_ADS7953SDBT_H_ */
