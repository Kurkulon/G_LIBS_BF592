#ifndef WAVEPACK_IMP_H__08_11_2023__17_02
#define WAVEPACK_IMP_H__08_11_2023__17_02

#include "types.h"
#include "fdct.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static const byte ulaw_0816_expenc[256] = {
	0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
	4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static const i8  adpcmima_0416_index_tab[16] = {-1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8};
static const u16 adpcmima_0416_stepsize_tab[89] = {
		 7,     8,     9,    10,    11,    12,    13,    14,    16,    17,
		19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
		50,    55,    60,    66,    73,    80,    88,    97,   107,   118,
	   130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
	   337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
	   876,   963,  1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
	  2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
	  5894,  6484,  7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
	 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WavePack_uLaw_12Bit(u16 *src, byte *dst, u16 len)
{
    byte sign, exponent, mantissa, sample_out;

	for (u32 i = len; i > 0; i--)
	{
		u16 sample_in = *(src++);

		sign = 0;

		if ((i16)sample_in < 0)
		{
			sign = 0x80;
			sample_in = -sample_in;
		};

		sample_in += 0x10;

		exponent = ulaw_0816_expenc[(sample_in >> 4) & 0xff];

		mantissa = (sample_in >> (exponent + 0)) & 0xf;

		sample_out = (sign | (exponent << 4) | mantissa);

		*(dst++) = sample_out;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WavePack_uLaw_16Bit(u16 *src, byte *dst, u16 len)
{
    byte sign, exponent, mantissa, sample_out;

	for (u32 i = len; i > 0; i--)
	{
		u16 sample_in = *(src++);

		sign = 0;

		if ((i16)sample_in < 0)
		{
			sign = 0x80;
			sample_in = -sample_in;
		};

		sample_in += 0x10;

		exponent = ulaw_0816_expenc[(sample_in >> 7) & 0xff];

		mantissa = (sample_in >> (exponent + 3)) & 0xf;

		sample_out = (sign | (exponent << 4) | mantissa);

		*(dst++) = sample_out;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WavePack_uLaw_FDCT(FDCT_DATA *s, byte *d, u16 len, byte scale)
{
    byte sign, exponent, mantissa, sample_out;

	for (; len > 0; len--)
	{
		u16 sample_in = (i16)((i32)(*(s++))>>scale);

		sign = 0;

		if ((i16)sample_in < 0)
		{
			sign = 0x80;
			sample_in = -sample_in;
		};

		//if (sample_in > ulaw_0816_clip) sample_in = ulaw_0816_clip;

		sample_in += 0x84;//ulaw_0816_bias;

		exponent = ulaw_0816_expenc[(sample_in >> 7) & 0xff];

		mantissa = (sample_in >> (exponent + 3)) & 0xf;

		sample_out = (sign | (exponent << 4) | mantissa);

		//if (sample_out == 0) sample_out = 2;

		*(d++) = sample_out;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WavePack_ADPCMIMA(u16 *src, byte* dst, u16 len)
{
    u16 stepsize = 7;     		/* Quantizer step size */
    i16 predictedSample = 0;	/* Output of ADPCM predictor */
    i8  index = 0;			/* Index into step size table */
    u8	newSample;			/* Result of encoding */

	byte bits = 0;

	for (u32 i = len; i > 0; i--)
	{
		i16 originalSample = *(src++);

		i16 dq = originalSample - predictedSample;
		if (dq >= 0) newSample = 0; else { newSample = 8; dq = -dq;}

		i16 diff = 0;

		if (dq >= stepsize) { newSample |= 4; dq -= stepsize; diff += stepsize; }; stepsize >>= 1;
		if (dq >= stepsize) { newSample |= 2; dq -= stepsize; diff += stepsize; }; stepsize >>= 1;
		if (dq >= stepsize) { newSample |= 1; dq -= stepsize; diff += stepsize; }; stepsize >>= 1;

		diff += stepsize;

		if (newSample & 8) diff = -diff;

		predictedSample += diff;

		if (predictedSample > 2047) predictedSample = 2047; else if (predictedSample < -2047) predictedSample = -2047;

		index += adpcmima_0416_index_tab[newSample];

		if (index < 0) index = 0; else if (index > 67) index = 67;

		stepsize = adpcmima_0416_stepsize_tab[index];

		bits |= newSample << ((i&1)*4);

		if (i&1) *(dst++) = bits, bits = 0;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // WAVEPACK_IMP_H__08_11_2023__17_02