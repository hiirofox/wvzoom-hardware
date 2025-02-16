#include "lpc.h"

/* Input : n elements of time doamin data
   Output: m lpc coefficients, excitation energy */

void LPC_Init(LPC_DATA *pd)
{
	for (int i = 0; i < MaxLPCDataLen; ++i)
		pd->aut[i] = pd->buf[i] = 0;
	for (int i = 0; i < MaxLPCCoeffsNum; ++i)
		pd->lpc[i] = 0;
}

float LPC_ProcDurbin(LPC_DATA *pd, const float *data, double *lpci, int n, int m)
{
	double error = 0.0;
	double epsilon = 0.0;
	/*
		for (int i = 0; i < n; ++i)
		{
			pd->buf[i] = data[i] + (0.5 - 0.5 * cosf((float)i * 2.0 * 3.141592653589 / n));
		}*/

	/* Calculate autocorrelation, p+1 lag coefficients */
	for (int j = 0; j <= m; ++j)
	{
		double accumulator = 0.0; // Double for accumulation depth
		for (int i = j; i < n; ++i)
		{
			accumulator += (double)data[i] * (double)data[i - j];
		}
		pd->aut[j] = accumulator;
	}

	/* Set our noise floor to about -100dB */
	error = pd->aut[0] * (1.0 + 1e-10);
	epsilon = 1e-9 * pd->aut[0] + 1e-10;

	/* Generate LPC coefficients from autocorrelation values */
	for (int i = 0; i < m; ++i)
	{
		double reflection = -pd->aut[i + 1];

		if (error < epsilon)
		{
			memset(pd->lpc + i, 0, (m - i) * sizeof(*pd->lpc));
			break; // Terminate early if error is below the threshold
		}

		/* Compute reflection coefficient */
		for (int j = 0; j < i; ++j)
		{
			reflection -= pd->lpc[j] * pd->aut[i - j];
		}
		reflection /= error;

		/* Update LPC coefficients and total error */
		pd->lpc[i] = reflection;
		for (int j = 0; j < i / 2; ++j)
		{
			double temp = pd->lpc[j];
			pd->lpc[j] += reflection * pd->lpc[i - 1 - j];
			pd->lpc[i - 1 - j] += reflection * temp;
		}
		if (i & 1)
		{ // Odd-indexed case
			pd->lpc[i / 2] += pd->lpc[i / 2] * reflection;
		}

		error *= 1.0 - reflection * reflection;
	}

	/* Slightly damp the filter */
	{
		const double g = 0.99; // Damping factor
		double damp = g;
		for (int j = 0; j < m; ++j)
		{
			pd->lpc[j] *= damp;
			damp *= g;
		}
	}

	/* Output LPC coefficients as floats */
	for (int j = 0; j < m; ++j)
	{
		lpci[j] = -(double)pd->lpc[j];
	}

	/* Return error value to determine the impulse strength later */
	return (float)error;
}
float LPC_ProcBurg(LPC_DATA *pd, const float *x, float *a, int N, int M)
{
	float *b1 = pd->buf;
	double *b2 = pd->aut;
	double *aa = pd->lpc;

	// (3)

	double gain = 0.0;
	for (size_t j = 0; j < N; j++)
	{
		gain += x[j] * x[j];
	}

	gain /= N;
	if (gain <= 0)
	{
		return 0.0; // warning empty
	}

	// (9)

	b1[0] = x[0];
	b2[N - 2] = x[N - 1];
	for (size_t j = 1; j < N - 1; j++)
	{
		b1[j] = b2[j - 1] = x[j];
	}

	for (int i = 0;; ++i)
	{
		// (7)

		double num = 0.0, denum = 0.0;
		for (int j = 0; j < N - i - 1; j++)
		{
			num += b1[j] * b2[j];
			denum += b1[j] * b1[j] + b2[j] * b2[j];
		}

		if (denum <= 0)
		{
			return 0.0; // warning ill-conditioned
		}

		a[i] = 2.0 * num / denum;

		// (10)

		gain *= 1.0 - a[i] * a[i];

		// (5)

		for (int j = 0; j + 1 <= i; j++)
		{
			a[j] = aa[j] - a[i] * aa[i - j - 1];
		}

		if (i == M - 1)
			break;

		// (8)  Watch out: i -> i+1

		for (int j = 0; j <= i; j++)
		{
			aa[j] = a[j];
		}

		for (int j = 0; j <= N - i - 2; j++)
		{
			b1[j] -= aa[i] * b2[j];
			b2[j] = b2[j + 1] - aa[i] * b1[j + 1];
		}
	}

	return gain;
}

void LPC_FilterInit(LPC_FILTER *filt)
{
	for (int i = 0; i < MaxLPCCoeffsNum; ++i)
	{
		filt->yl[i] = 0;
		filt->yr[i] = 0;
	}
	filt->pos = 0;
}

void LPC_FilterPredictStereo(LPC_FILTER *filt,
							 const float *inl, const float *inr,
							 float *outl, float *outr, double *a,
							 int numSamples, int numCoeffs,
							 float errorv)
{
	for (int i = 0; i < numSamples; ++i)
	{
		double suml = 0;
		double sumr = 0;
		int start = MaxLPCCoeffsNum + filt->pos;
		for (int k = 0; k < numCoeffs; ++k)
		{
			int n = (start - k - 1) % MaxLPCCoeffsNum;
			suml += filt->yl[n] * a[k];
			sumr += filt->yr[n] * a[k];
		}
		filt->yl[filt->pos] = inl[i] * errorv * 0.0005 + suml;
		filt->yr[filt->pos] = inr[i] * errorv * 0.0005 + sumr;
		outl[i] = filt->yl[filt->pos] * 5.0;
		outr[i] = filt->yr[filt->pos] * 5.0;
		if (filt->yl[filt->pos] > 1.0)
			filt->yl[filt->pos] = 1.0;
		else if (filt->yl[filt->pos] < -1.0)
			filt->yl[filt->pos] = -1.0;
		if (filt->yr[filt->pos] > 1.0)
			filt->yr[filt->pos] = 1.0;
		else if (filt->yr[filt->pos] < -1.0)
			filt->yr[filt->pos] = -1.0;

		filt->pos++;
		if (filt->pos >= MaxLPCCoeffsNum)
			filt->pos = 0;
	}
}
void LPC_FilterCompensationStereo(LPC_FILTER *filt,
								  const float *inl, const float *inr,
								  float *outl, float *outr, float *a,
								  int numSamples, int numCoeffs)
{
	for (int i = 0; i < numSamples; ++i)
	{
		float suml = 0;
		float sumr = 0;
		filt->yl2[filt->pos] = inl[i];
		filt->yr2[filt->pos] = inr[i];
		int start = MaxLPCCoeffsNum + filt->pos - 1;
		for (int j = 0; j < numCoeffs; ++j)
		{
			int n = (start - j) % MaxLPCCoeffsNum;
			suml += a[j] * filt->yl2[n];
			sumr += a[j] * filt->yr2[n];
		}
		outl[i] = inl[i] - suml;
		outr[i] = inr[i] - sumr;
		filt->pos++;
		if (filt->pos >= MaxLPCCoeffsNum)
			filt->pos = 0;
	}
}
