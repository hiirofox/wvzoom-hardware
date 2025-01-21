#include "printf.h"

#define IS_DEBUG 1

UART_HandleTypeDef *phuart = NULL;
char xxx_tmpbuf[1024];

void uart_setUartHandle(UART_HandleTypeDef *huart)
{
	phuart = huart;
}

void uart_printf(const char *fmt, ...)
{
#ifdef IS_DEBUG
	if (phuart == NULL)
		return;
	va_list args;
	va_start(args, fmt);
	int size = vsnprintf(xxx_tmpbuf, 1024, fmt, args);
	va_end(args);
	HAL_UART_Transmit(phuart, (const unsigned char *)xxx_tmpbuf, size, 0xFFFF);
#endif
}
