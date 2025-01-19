#ifndef __LMODEL_H750_CODEC_
#define __LMODEL_H750_CODEC_

#include <stdint.h>
#include <stdlib.h>
#include "stm32h7xx_hal.h"
#include "printf.h"

#define SampleRate 48000
#define ActualSampleRate 46875 // 实际上采样率只有这么点

#define MaxHandleNum 32

void codecInit(I2S_HandleTypeDef *p1, I2S_HandleTypeDef *p2, I2S_HandleTypeDef *p3);
void codecWaitForAllBufferPrepare();
int32_t *codecGetRx1Ptr();
int32_t *codecGetRx2Ptr();
int32_t *codecGetRx3Ptr();
int32_t *codecGetTx1Ptr();
int32_t *codecGetTx2Ptr();
int32_t *codecGetTx3Ptr();
int codecGetNumSamples();

void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s);

#endif