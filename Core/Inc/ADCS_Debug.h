/*
 * ADCS_Debug.h
 *
 *  Created on: Feb 19, 2026
 *      Author: Antarikchya
 */


#ifndef INC_ADCS_DEBUG_H_
#define INC_ADCS_DEBUG_H_

#include "main.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

extern UART_HandleTypeDef huart7;

int bufferSize(char *buff);
void myDebug(const char *fmt, ...);

#endif /* INC_ADCS_DEBUG_H_ */
