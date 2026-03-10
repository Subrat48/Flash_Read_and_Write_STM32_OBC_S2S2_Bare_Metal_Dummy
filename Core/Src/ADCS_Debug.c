/*
 * ADCS_Debug.c
 *
 *  Created on: Feb 19, 2026
 *      Author: Antarikchya
 */


#include "ADCS_Debug.h"

void myDebug(const char *fmt, ...) {
	static char temp[100];
	va_list args;
	va_start(args, fmt);
	vsnprintf(temp, sizeof(temp), fmt, args);
	va_end(args);
	int len = bufferSize(temp);
	HAL_UART_Transmit(&huart7, (uint8_t*) temp, len, 1000);
}

int bufferSize(char *buff) {
	int i = 0;
	while (*buff++ != '\0')
		i++;
	return i;
}
