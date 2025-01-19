#ifndef __LMODEL_PRINTF_
#define __LMODEL_PRINTF_

#include <stm32h7xx_hal.h>
#include <stdio.h>
#include <stdarg.h>

void uart_setUartHandle(UART_HandleTypeDef *huart);
void uart_printf(const char *fmt, ...);

#endif