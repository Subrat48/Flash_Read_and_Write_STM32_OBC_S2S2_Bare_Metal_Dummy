/*
 * ADS7953SDBT.c
 *
 * Created on: Mar 10, 2026
 * Author: Subrat
 */

#include "ADS7953SDBT.h"
#include "math.h"

uint8_t adc1_data[2];
/* uint8_t adc2_data[2]; */

void ADC1_Enable() {
    HAL_GPIO_WritePin(GPIOH, ADC1_CS_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

void ADC1_Disable() {
    HAL_GPIO_WritePin(GPIOH, ADC1_CS_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
}

void ADC_Operate(uint16_t *adc1_channels) {
    AUTO_2_Select_ADC1(SELECT_ADC);
    AUTO_2_Program_ADC1();

    for (int x = 0; x < 16; x++) {
        AUTO_2_Select_ADC1(READ_ADC);
        ADC_CombineData(adc1_channels);
    }
}

void AUTO_2_Select_ADC1(operation_modes mode) {
    uint8_t command[2];
    uint16_t temp;
    switch (mode) {
    case SELECT_ADC:
        temp = AUTO_2_MODE;
        command[0] = temp >> 8;
        command[1] = temp & 0xff;
        ADC1_Enable();
        HAL_Delay(1);
        HAL_SPI_Transmit(&hspi5, command, 2, 1000);
        HAL_Delay(1);
        ADC1_Disable();
        HAL_Delay(1);
        break;

    case READ_ADC:
        temp = AUTO_2_MODE_NO_RST;
        command[0] = temp >> 8;
        command[1] = temp & 0xff;
        ADC1_Enable();
        HAL_Delay(1);
        HAL_SPI_TransmitReceive(&hspi5, command, adc1_data, 2, 1000);
        HAL_Delay(1);
        ADC1_Disable();
        HAL_Delay(1);
        break;
    default:
        break;
    }
}

void AUTO_2_Program_ADC1() {
    uint8_t command[2];
    uint16_t temp;

    temp = AUTO_2_PROGRAM_ADC1;
    command[0] = temp >> 8;
    command[1] = temp & 0xff;

    ADC1_Enable();
    HAL_Delay(1);
    HAL_SPI_Transmit(&hspi5, command, 2, 1000);
    HAL_Delay(1);
    ADC1_Disable();
    HAL_Delay(1);
}

void Continue_Operaion(uint8_t *data) {
    uint8_t command[2];
    uint16_t temp;
    uint8_t rx_data[2];

    temp = CONTINUE_OPERATION;
    command[0] = temp >> 8;
    command[1] = temp & 0xff;

    ADC1_Enable();
    HAL_SPI_Transmit(&hspi5, command, 2, 1000);
    HAL_SPI_Receive(&hspi5, rx_data, 2, 1000);
    data[0] = rx_data[0];
    data[1] = rx_data[1];
    ADC1_Disable();
    HAL_Delay(1);
}

void ADC_CombineData(uint16_t *adc1_channels) {
    static int i = 0;

    if (i < 16) {
        adc1_channels[i] = ((adc1_data[0] << 8) | adc1_data[1]);
        i++;
        if (i == 16) {
            for (int x = 0; x < 16; x++) {
                adc1_channels[x] = adc1_channels[x] & 0x0fff;
            }
            i = 0;
        }
    }
}

void ADC1_RawConv_Data(float *adc1_conv_buf, uint16_t *adc1_buf) {
    for (int x = 0; x < 16; x++) {
        // Enforced explicit floating point math
        adc1_conv_buf[x] = (2.5f * (float)adc1_buf[x]) / 4095.0f;
    }
}

void ADC1_Volt_Temp_Conv(float *adc1_conv_buf, float *temp_buf) {
    float root = 0.0f;

    for (int i = 0; i < 8; i++) {
        temp_buf[i] = (adc1_conv_buf[i] * (1100.0f + 931.0f)) / 931.0f;
    }

    for (int i = 8; i < 16; i++) {
        if (i == 9) {
            // Battery temperature channel (Safe boundaries to prevent math crashes)
            if (adc1_conv_buf[i] < 0.05f || adc1_conv_buf[i] >= 2.45f) {
                temp_buf[i] = 0.0f;
            } else {
                // Explicitly use logf() for single-precision embedded math
                float res = (adc1_conv_buf[i] * 10000.0f) / (2.5f - adc1_conv_buf[i]);
                float tempK = (3976.0f * 298.0f) / (3976.0f - (298.0f * logf(10000.0f / res)));
                temp_buf[i] = (tempK - 273.15f) * 100.0f;
            }
        } else {
            if (adc1_conv_buf[i] < 0.05f) {
                temp_buf[i] = 0.0f;
            } else {
                float inner = (5.506f * 5.506f) + (4.0f * 0.00176f * (870.6f - (adc1_conv_buf[i] * 1000.0f)));
                if (inner > 0.0f) {
                    root = sqrtf(inner);
                    temp_buf[i] = ((((5.506f - root) / (2.0f * (-0.00176f))) + 30.0f)) * 100.0f;
                } else {
                    temp_buf[i] = 0.0f;
                }
            }
        }
    }
}
