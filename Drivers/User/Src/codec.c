#include "codec.h"


#define buflen 768
#define IIS_TYPE int32_t
IIS_TYPE i2s1_tx_buf[buflen * 2];
IIS_TYPE i2s1_rx_buf[buflen * 2];
IIS_TYPE i2s2_tx_buf[buflen * 2];
IIS_TYPE i2s2_rx_buf[buflen * 2];
IIS_TYPE i2s3_tx_buf[buflen * 2];
IIS_TYPE i2s3_rx_buf[buflen * 2];
IIS_TYPE *wavbuf1 = i2s1_tx_buf;
IIS_TYPE *recbuf1 = i2s1_rx_buf;
IIS_TYPE *wavbuf2 = i2s2_tx_buf;
IIS_TYPE *recbuf2 = i2s2_rx_buf;
IIS_TYPE *wavbuf3 = i2s3_tx_buf;
IIS_TYPE *recbuf3 = i2s3_rx_buf;

int I2S1_ISFILL_OK = 0;
int I2S2_ISFILL_OK = 0;
int I2S3_ISFILL_OK = 0;

I2S_HandleTypeDef *phi2s1 = NULL;
I2S_HandleTypeDef *phi2s2 = NULL;
I2S_HandleTypeDef *phi2s3 = NULL;

void codecInit(I2S_HandleTypeDef *p1, I2S_HandleTypeDef *p2, I2S_HandleTypeDef *p3)
{
	phi2s1 = p1;
	phi2s2 = p2;
	phi2s3 = p3;
	HAL_I2SEx_TransmitReceive_DMA(phi2s1, (uint16_t *)i2s1_tx_buf, (uint16_t *)i2s1_rx_buf, buflen * 2);
	HAL_I2SEx_TransmitReceive_DMA(phi2s2, (uint16_t *)i2s2_tx_buf, (uint16_t *)i2s2_rx_buf, buflen * 2);
	HAL_I2SEx_TransmitReceive_DMA(phi2s3, (uint16_t *)i2s3_tx_buf, (uint16_t *)i2s3_rx_buf, buflen * 2);
	;
}

void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s == phi2s1)
	{
		wavbuf1 = &i2s1_tx_buf[0];
		recbuf1 = &i2s1_rx_buf[0];
		I2S1_ISFILL_OK = 1;
	}
	else if (hi2s == phi2s2)
	{
		wavbuf2 = &i2s2_tx_buf[0];
		recbuf2 = &i2s2_rx_buf[0];
		I2S2_ISFILL_OK = 1;
	}
	else if (hi2s == phi2s3)
	{
		wavbuf3 = &i2s3_tx_buf[0];
		recbuf3 = &i2s3_rx_buf[0];
		I2S3_ISFILL_OK = 1;
	}
}
void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s == phi2s1)
	{
		wavbuf1 = &i2s1_tx_buf[buflen];
		recbuf1 = &i2s1_rx_buf[buflen];
		I2S1_ISFILL_OK = 1;
	}
	else if (hi2s == phi2s2)
	{
		wavbuf2 = &i2s2_tx_buf[buflen];
		recbuf2 = &i2s2_rx_buf[buflen];
		I2S2_ISFILL_OK = 1;
	}
	else if (hi2s == phi2s3)
	{
		wavbuf3 = &i2s3_tx_buf[buflen];
		recbuf3 = &i2s3_rx_buf[buflen];
		I2S3_ISFILL_OK = 1;
	}
}

void codecWaitForAllBufferPrepare()
{
	for (volatile int tmp = 0;; tmp++)
	{
		volatile int flag = 1;
		if (phi2s1 != NULL && I2S1_ISFILL_OK == 0)
			flag = 0;
		if (phi2s2 != NULL && I2S2_ISFILL_OK == 0)
			flag = 0;
		if (phi2s3 != NULL && I2S3_ISFILL_OK == 0)
			flag = 0;
		if (flag)
			break;
		__asm volatile("nop");
		__asm volatile("nop");
	}
	I2S1_ISFILL_OK = 0;
	I2S2_ISFILL_OK = 0;
	I2S3_ISFILL_OK = 0;
}

int32_t *codecGetTx1Ptr() { return wavbuf1; }
int32_t *codecGetTx2Ptr() { return wavbuf2; }
int32_t *codecGetTx3Ptr() { return wavbuf3; }
int32_t *codecGetRx1Ptr() { return recbuf1; }
int32_t *codecGetRx2Ptr() { return recbuf2; }
int32_t *codecGetRx3Ptr() { return recbuf3; }

int codecGetNumSamples()
{
	return buflen;
}