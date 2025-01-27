#pragma once

#include <math.h>
#include <stdlib.h>
#include <string.h>
// #include <malloc.h>
#include "complex.h"
// #include "fft.h"

#define MaxLPCDataLen 512
#define MaxLPCCoeffsNum 64

typedef struct
{
	float buf[MaxLPCDataLen];
	double aut[MaxLPCDataLen];
	double lpc[MaxLPCCoeffsNum];
} LPC_DATA;

void LPC_Init(LPC_DATA *pd);
float LPC_ProcDurbin(LPC_DATA *pd, const float *data, double *lpci, int n, int m);
float LPC_ProcBurg(LPC_DATA *pd, const float *x, float *a, int N, int M);

typedef struct
{
	float yl[MaxLPCCoeffsNum];
	float yr[MaxLPCCoeffsNum];
	float yl2[MaxLPCCoeffsNum];
	float yr2[MaxLPCCoeffsNum];
	int pos;
} LPC_FILTER;

void LPC_FilterInit(LPC_FILTER *filt);
void LPC_FilterPredictStereo(LPC_FILTER *filt, const float *inl, const float *inr,
							 float *outl, float *outr, double *coeffs, int numSamples, int numCoeffs, float errorv);
void LPC_FilterCompensationStereo(LPC_FILTER *filt, const float *inl, const float *inr,
								  float *outl, float *outr, float *coeffs, int numSamples, int numCoeffs);
