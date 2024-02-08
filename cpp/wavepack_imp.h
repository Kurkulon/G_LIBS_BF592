#ifndef WAVEPACK_IMP_H__08_11_2023__17_02
#define WAVEPACK_IMP_H__08_11_2023__17_02

#include "types.h"
#include "fdct.h"
#include "mdct.h"

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

const u16 ulaw_0812_expdec[8] = { 0, 16, 16 * 3, 16 * 7, 16 * 15, 16 * 31, 16 * 63, 16 * 127 };
const u16 ulaw_0816_expdec[8] = { 0, 132, 132 * 3, 132 * 7, 132 * 15, 132 * 31, 132 * 63, 132 * 127 };

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

static void WaveUnpack_uLaw_12Bit(byte* src, u16* dst, u16 len)
{
	for (u32 i = len; i > 0; i--)
	{
		byte sample_in = *(src++);
		byte exponent = (byte)((sample_in >> 4) & 7);
		u16	 sample_out = ulaw_0812_expdec[exponent];
		sample_out += (u16)((sample_in & 0xF) << (exponent + 0));
		if ((sample_in & 0x80) != 0) sample_out = (u16)-sample_out;
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

		sample_in += 0x84;

		exponent = ulaw_0816_expenc[(sample_in >> 7) & 0xff];

		mantissa = (sample_in >> (exponent + 3)) & 0xf;

		sample_out = (sign | (exponent << 4) | mantissa);

		*(dst++) = sample_out;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WaveUnpack_uLaw_16Bit(byte* src, u16* dst, u16 len)
{

	//byte sign, exponent, mantissa;

	for (u32 i = len; i > 0; i--)
	{
		byte sample_in = *(src++);
		byte exponent = (byte)((sample_in >> 4) & 7);
		u16	 sample_out = ulaw_0816_expdec[exponent];
		sample_out += (u16)((sample_in & 0xF) << (exponent + 3));
		if ((sample_in & 0x80) != 0) sample_out = (u16)-sample_out;
		*(dst++) = sample_out;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WavePack_ADPCMIMA(u16 *src, byte* dst, u16 len)
{
    u16 stepsize = 7;     		/* Quantizer step size */
    i16 predictedSample = 0;	/* Output of ADPCM predictor */
    i8  index = 0;				/* Index into step size table */
    u8	newSample;				/* Result of encoding */

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

		if (predictedSample > 32767) predictedSample = 32767; else if (predictedSample < -32767) predictedSample = -32767;

		index += adpcmima_0416_index_tab[newSample];

		if (index < 0) index = 0; else if (index > (ArraySize(adpcmima_0416_stepsize_tab) - 1)) index = ArraySize(adpcmima_0416_stepsize_tab) - 1;

		stepsize = adpcmima_0416_stepsize_tab[index];

		bits |= newSample << ((i&1)*4);

		if (i&1) *(dst++) = bits, bits = 0;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WaveUnpack_ADPCMIMA(byte* src, u16* dst, u16 len)
{
	i32	 newSample = 0; // predictedSample
	u16	 stepsize = 7;
	i8 index = 0;

	for (u32 i = 0; i < len; i++)
	{
		u32 j = i / 2;
		byte originalSample = (byte)((src[j] >> ((i&1)*4)) & 0xF);

		i32 diff = 0;

		if ((originalSample & 4) != 0) diff += stepsize;
		if ((originalSample & 2) != 0) diff += stepsize >> 1;
		if ((originalSample & 1) != 0) diff += stepsize >> 2;

		diff += stepsize >> 3;

		if ((originalSample & 8) != 0) diff = -diff;

		newSample += diff;

		if (newSample > 32767) newSample = 32767;
		else if (newSample < -32767) newSample = -32767;

		index += adpcmima_0416_index_tab[originalSample];

		if (index < 0) index = 0; else if (index > (ArraySize(adpcmima_0416_stepsize_tab)-1)) index = ArraySize(adpcmima_0416_stepsize_tab)-1;

		stepsize = adpcmima_0416_stepsize_tab[index];

		*(dst++) = (i16)newSample;

	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WavePack_uLaw_FDCT(FDCT_DATA* src, byte* dst, u16 len, u16 scale)
{
	byte sign, exponent, mantissa, sample_out;

	for (; len > 0; len--)
	{
		u16 sample_in = (i16)((i32)(*(src++)) >> scale);

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

		*(dst++) = sample_out;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u16 WavePack_FDCT_Quant(FDCT_DATA* src, u16 packLen, u16 shift, u16* const scale)
{
	packLen = (packLen + 1) & ~1;

	FDCT_DATA max = 0;

	for (u32 i = 1; i < packLen; i++)
	{
		FDCT_DATA t = src[i];

		if (t < 0) t = -t;

		if (t > max) max = t;
	};

	FDCT_DATA* p = src + packLen - 1;
	FDCT_DATA lim = max;

	if (lim < 64) lim = 64;

	lim = (i32)lim >> shift;

	*scale = 0;

	if (src[0] > max) max = src[0];

	while (max > 32000) { max /= 2; *scale += 1; };

	u32 xx = 8 << *scale;

	if ((u32)lim < xx) lim = xx;

	for (u16 i = packLen; i > 2; i--)
	{
		FDCT_DATA t = *(p--);

		if (t < 0) t = -t;

		if (t > lim)
		{
			packLen = i;
			break;
		};

		packLen -= 1;
	};

	return (packLen + 1) & ~1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WavePack_FDCT_Transform(i16* s, FDCT_DATA *fdct_w)
{
	for (u32 n = 0; n < FDCT_N; n++) fdct_w[n] = *(s++);

	FastDctLee_transform(fdct_w, FDCT_LOG2N);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u16 WavePack_FDCT(i16* src, byte* dst, u16 len, u16 shift, u16 OVRLAP, u16 maxPackLen, u16 *packedLen)
{
	if (src == 0 || dst == 0 || len < FDCT_N || packedLen == 0) return 0;

	u16 packLen = 0;
	u16 index = 0;
	u16 scale = 0;
	u16 wpLen = 0;

	FDCT_DATA fdct_w[FDCT_N];

	if (shift < 1) shift = 1;
	if (maxPackLen > FDCT_N) maxPackLen = FDCT_N;

	for (;(index + FDCT_N) <= len; index += FDCT_N - OVRLAP)
	{
		WavePack_FDCT_Transform(src + index, fdct_w);

		packLen = WavePack_FDCT_Quant(fdct_w, FDCT_N, shift, &scale);

		PackDCT* pdct = (PackDCT*)(dst + wpLen);

		WavePack_uLaw_FDCT(fdct_w, pdct->data, packLen, scale);

		pdct->len = (byte)packLen;
		pdct->scale = (byte)scale;

		wpLen += 2 + packLen;
	};

	if (packedLen != 0) *packedLen = wpLen;

	return index + OVRLAP;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef _MSC_VER

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u16 WaveUnpack_FDCT(byte* src, i16* dst, u16 srcLen, u16 OVRLAP)
{
	if (src == 0 || dst == 0 || srcLen < 2) return 0;

	FDCTDATA vect[FDCT_N];
	FDCTDATA temp[FDCT_N];

	//u32 packIndex = 0;

	//u32 size = srcLen;
	u16 index = 0;

	for (; srcLen > 0; index += FDCT_N - OVRLAP)
	{
		byte packDctLen	= *(src++);
		byte scale			= *(src++);

		srcLen -= 2;

		if (srcLen < packDctLen || srcLen == 0) break;

		for (int i = 0; i < packDctLen; i++)
		{
			byte sample_in = *(src++);
			byte exponent = (byte)((sample_in >> 4) & 7);
			u16 sample_out = ulaw_0816_expdec[exponent];

			sample_out += (u16)((sample_in & 0xF) << (exponent + 3));

			if ((sample_in & 0x80) != 0) sample_out = (u16)-sample_out;

			vect[i] = (i16)sample_out;
		};

		for (u32 i = packDctLen; i < FDCT_N; i++) vect[i] = 0;

		//packIndex += 2 + packDctLen;

		FDCT_Inverse(vect, temp, FDCT_LOG2N, (float)(2 << scale) / FDCT_N);

		if (index < OVRLAP)
		{
			for (u32 i = 0; i < FDCT_N; i++) *(dst++) = (i16)(LIM(vect[i], -32767, 32767));
		}
		else
		{
			for (u16 i = 0; i < OVRLAP; i++)
			{
				float t = (dst[0] * (OVRLAP - i) + vect[i] * (i + 1)) / (OVRLAP + 1);
				*(dst++) = (i16)(LIM(t, -32767, 32767));
			};

			for (u16 i = OVRLAP; i < FDCT_N; i++) *(dst++) = (i16)(LIM(vect[i], -32767, 32767));

			//u16 h = OVRLAP / 2;
			//dst += h;
			//float t = LIM(vect[h], -32767, 32767);
			//*(dst++) = (i16)((dst[0]+t)/2);

			//for (u16 i = h+1; i < FDCT_N; i++) *(dst++) = (i16)(LIM(vect[i], -32767, 32767));
		};

		dst -= OVRLAP;

		if (srcLen >= packDctLen) srcLen -= packDctLen; else srcLen = 0;
	};

	return index + OVRLAP;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 WavePack_MDCT(MDCT_LookUp* init, i16* src, MDCT_DATA* dst, u16 len, u16 shift, u16* packedLen)
{
	if (init == 0 || src == 0 || dst == 0) return 0;

	u16 packLen = 0;
	u16 index = 0;
	u16 scale = 0;
	u16 wpLen = 0;
	u16 N = init->n;
	u16 OVRLAP = N >> 1;

	//MDCT_DATA fdct_w[FDCT_N];

	for (;(index + N) <= len; index += N - OVRLAP)
	{
		for (u16 i = 0; i < N; i++) init->temp1[i] = src[index + i];
		//mdct_window(init, src + index, init->temp1);
		mdct_forward(init, init->temp1, dst);

		dst += OVRLAP;
	};

	//if (packedLen != 0) *packedLen = wpLen;

	return index + OVRLAP;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 WaveUnpack_MDCT(MDCT_LookUp* init, MDCT_DATA* src, i16* dst, u16 len)
{
	if (init == 0 || src == 0 || dst == 0) return 0;

	u16 packLen = 0;
	u16 index = 0;
	u16 scale = 0;
	u16 wpLen = 0;
	u16 N = init->n;
	u16 OVRLAP = N >> 1;

	MDCT_DATA* temp1 = init->temp1;
	MDCT_DATA* temp2 = init->temp2;

	//MDCT_DATA fdct_w[FDCT_N];
	mdct_backward(init, src, temp1);

	src += OVRLAP;
	index += N - OVRLAP;

	for (u16 i = 0; i < OVRLAP; i++) *(dst++) = (i16)temp1[i];

	MDCT_DATA* t = temp1; temp1 = temp2; temp2 = t;

	for (;(index + N) <= len; index += N - OVRLAP)
	{
		mdct_backward(init, src, temp1);
		//mdct_window(init, temp1, temp1);

		for (u16 i = 0; i < OVRLAP; i++) *(dst++) = (i16)((temp1[i] + temp2[i + OVRLAP])/2);

		t = temp1; temp1 = temp2; temp2 = t;

		src += OVRLAP;
	};

	//if (packedLen != 0) *packedLen = wpLen;

	return index + OVRLAP;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // #ifdef _MSC_VER

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#endif // WAVEPACK_IMP_H__08_11_2023__17_02
