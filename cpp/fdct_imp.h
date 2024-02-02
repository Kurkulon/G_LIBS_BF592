#ifndef FDCT_IMP_H__08_11_2023__15_16
#define FDCT_IMP_H__08_11_2023__15_16

/* 
 * Fast discrete cosine transform algorithms (C)
 * 
 * Copyright (c) 2021 Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/fast-discrete-cosine-transform-algorithms
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

//#pragma optimize_for_speed

#include <math.h>
#include "fdct.h"

#ifndef FDCT_LOG2N
#define FDCT_LOG2N 6
#endif

#ifndef FDCT_N
#define FDCT_N (1UL<<FDCT_LOG2N)
#endif

#ifndef M_PI
#define M_PI 3.141592653
#endif 


#ifndef FDCT_FLOAT

#ifndef FDCT_TRIGBITS
#define FDCT_TRIGBITS 10
#endif

#define FDCT_FLOAT(x)	((int)((x)*(1<<FDCT_TRIGBITS)+.5))
#define FDCT_MULT(x)	(((x)+(1<<(FDCT_TRIGBITS-1)))>>FDCT_TRIGBITS)

#else

typedef float FDCT_DATA;
typedef float FDCT_TRIG;

#define FDCT_FLOAT(x) (x)
#define FDCT_MULT(x) ((x)+0.5f)

#endif 

static void forwardTransform(FDCT_DATA vector[restrict], FDCT_DATA temp[restrict], u16 len);
static void inverseTransform(FDCT_DATA vector[restrict], FDCT_DATA temp[restrict], u16 len);
static void forwardTransform_v2(FDCT_DATA vector[restrict], FDCT_DATA temp[restrict], u16 len);
static void forwardTransform_v3(FDCT_DATA vector[restrict], FDCT_DATA temp[restrict], u16 len);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static FDCT_TRIG fdct_trig[FDCT_N] = {0};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FDCT_Init()
{
	//u32 len = ArraySize(fdct_trig)/2;

	for (u32 len = 1; len < ArraySize(fdct_trig); len *= 2)
	{
		FDCT_TRIG *trig = fdct_trig + len;

		for (u32 i = 0; i < len; i++)
		{
			trig[i] = FDCT_FLOAT(0.5f / cos((i + 0.5) * M_PI / (len*2)));
		};
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// DCT type II, unscaled. Algorithm by Byeong Gi Lee, 1984.
// See: http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.118.3056&rep=rep1&type=pdf#page=34

bool FastDctLee_transform(FDCT_DATA vector[], u16 log2n)
{
	if (log2n < 3 || log2n > FDCT_LOG2N) return false;  // Length is not power of 2

	FDCT_DATA temp[FDCT_N];

	u16 len = 1UL<<log2n;

	forwardTransform_v3(vector, temp, len);

	#ifndef FDCT_FLOAT
		//for (u16 i = 0; i < len; i++) vector[i] >>= log2n-3;
	#else
		//for (u16 i = 0; i < len; i++) vector[i] /= len/8;
	#endif

	vector[0] /= 2;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void forwardTransform2(FDCT_DATA vector[restrict])
{
	FDCT_DATA x = vector[0];
	FDCT_DATA y = vector[1];

	vector[0] = x + y;
	vector[1] = FDCT_MULT((x - y) * fdct_trig[1]); 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void forwardTransform4(FDCT_DATA vector[restrict] )
{
	FDCT_DATA temp0 =				vector[0] + vector[3];
	FDCT_DATA temp2 =	FDCT_MULT((	vector[0] - vector[3]	) * fdct_trig[2]);

	FDCT_DATA temp1 =				vector[1] + vector[2];
	FDCT_DATA temp3 =	FDCT_MULT((	vector[1] - vector[2]	) * fdct_trig[3]);

	vector[0] = temp0 + temp1;
	vector[2] = FDCT_MULT((temp0 - temp1) * fdct_trig[1]); 
	vector[3] = FDCT_MULT((temp2 - temp3) * fdct_trig[1]); 
	vector[1] = temp2 + temp3 + vector[3]; 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void forwardTransform8(FDCT_DATA vector[restrict])
{
	FDCT_DATA temp0 =				vector[0] + vector[7];
	FDCT_DATA temp4 = FDCT_MULT((	vector[0] - vector[7]) * fdct_trig[4]); // / (cos((i + 0.5) * M_PI / len) * 2);

	FDCT_DATA temp1 =				vector[1] + vector[6];
	FDCT_DATA temp5 = FDCT_MULT((	vector[1] - vector[6]) * fdct_trig[5]); // / (cos((i + 0.5) * M_PI / len) * 2);

	FDCT_DATA temp2 =				vector[2] + vector[5];
	FDCT_DATA temp6 = FDCT_MULT((	vector[2] - vector[5]) * fdct_trig[6]); // / (cos((i + 0.5) * M_PI / len) * 2);

	FDCT_DATA temp3 =				vector[3] + vector[4];
	FDCT_DATA temp7 = FDCT_MULT((	vector[3] - vector[4]) * fdct_trig[7]); // / (cos((i + 0.5) * M_PI / len) * 2);

	FDCT_DATA t0 =				temp0 + temp3;
	FDCT_DATA x = FDCT_MULT((	temp0 - temp3) * fdct_trig[2]);

	FDCT_DATA t1 =				temp1 + temp2;
	FDCT_DATA y = FDCT_MULT((	temp1 - temp2) * fdct_trig[3]);

	vector[0] =				t0 + t1;								//temp0
	vector[4] = FDCT_MULT((	t0 - t1) * fdct_trig[1]);	// temp2

	vector[6] = FDCT_MULT((	x - y) * fdct_trig[1]); 
	vector[2] =				x + y + vector[6];	// temp1

	t0 =			temp4 + temp7;
	x = FDCT_MULT((	temp4 - temp7) * fdct_trig[2]);

	t1 =			temp5 + temp6;
	y = FDCT_MULT((	temp5 - temp6) * fdct_trig[3]);

	temp4 =				t0 + t1;
	temp6 = FDCT_MULT((	t0 - t1) * fdct_trig[1]); 

	t0 = FDCT_MULT((	x - y) * fdct_trig[1]); 
	temp5 =				x + y + t0; 

	vector[1] = temp4 + temp5;
	vector[3] = temp5 + temp6;
	vector[5] = temp6 + t0;
	vector[7] = t0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void forwardTransform(FDCT_DATA vector[restrict], FDCT_DATA temp[restrict], u16 len)
{
	//if (len == 1) return;

	u16 halfLen = len / 2;

	FDCT_TRIG *trig = fdct_trig + halfLen;

	for (u16 i = 0; i < halfLen; i++)
	{
		FDCT_DATA x = vector[i];
		FDCT_DATA y = vector[len - 1 - i];

		temp[i] = x + y;
		temp[i + halfLen] = FDCT_MULT((x - y) * trig[i]); // / (cos((i + 0.5) * M_PI / len) * 2);
	};

	if (halfLen != 8)
	{
		forwardTransform(temp,			vector, halfLen);
		forwardTransform(temp+halfLen,	vector, halfLen);
	}
	else
	{
		forwardTransform8(temp			);
		forwardTransform8(temp+halfLen	);
	}

	for (u16 i = 0; i < halfLen - 1; i++)
	{
		vector[i * 2 + 0] = temp[i];
		vector[i * 2 + 1] = temp[i + halfLen] + temp[i + halfLen + 1];
	};

	vector[len - 2] = temp[halfLen - 1];
	vector[len - 1] = temp[len - 1];
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void forwardTransform_v2(FDCT_DATA vector[restrict], FDCT_DATA temp[restrict], u16 len)
{
	//u16 halfLen = len / 2;

	FDCT_DATA *vec = temp;
	FDCT_DATA *tmp = vector;

    for (u16 halfLen = len / 2; halfLen > 0; halfLen /= 2)
    {
		FDCT_TRIG *trig = fdct_trig + halfLen;

		FDCT_DATA *t = tmp; tmp = vec; vec = t;

		for (u16 off = 0; off < len; off += halfLen*2)
        {
            for (u16 i = 0; i < halfLen; i++)
            {
                FDCT_DATA x = vec[off + i];
                FDCT_DATA y = vec[off + halfLen*2 - 1 - i];
                tmp[off + i] = x + y;
                tmp[off + i + halfLen] = FDCT_MULT((x - y) * trig[i]);
            };
        };
        
    };

    for (u16 halfLen = 1; halfLen < len; halfLen *= 2)
    {
        for (u16 off = 0; off < len; off += halfLen*2)
        {
            for (u16 i = 0; i < halfLen - 1; i++)
            {
                vec[off + i * 2 + 0] = tmp[off + i];
                vec[off + i * 2 + 1] = tmp[off + i + halfLen] + tmp[off + i + halfLen + 1];
            };
        
            vec[off + halfLen*2 - 2] = tmp[off + halfLen - 1];
            vec[off + halfLen*2 - 1] = tmp[off + halfLen*2 - 1];
        };

		FDCT_DATA *t = tmp; tmp = vec; vec = t;
    };
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void forwardTransform_v3(FDCT_DATA vector[restrict], FDCT_DATA temp[restrict], u16 len)
{
	//u16 halfLen = len / 2;

	FDCT_DATA *vec = temp;
	FDCT_DATA *tmp = vector;

    for (u16 halfLen = len / 2; halfLen > 4; halfLen /= 2)
    {
		FDCT_TRIG *trig = fdct_trig + halfLen;

		FDCT_DATA *t = tmp; tmp = vec; vec = t;

		for (u16 off = 0; off < len; off += halfLen*2)
        {
            for (u16 i = 0; i < halfLen; i++)
            {
                FDCT_DATA x = vec[off + i];
                FDCT_DATA y = vec[off + halfLen*2 - 1 - i];
                tmp[off + i] = x + y;
                tmp[off + i + halfLen] = FDCT_MULT((x - y) * trig[i]);
            };
        };
        
    };

	for (u16 i = 0; i < len; i += 8) forwardTransform8(tmp+i);

    for (u16 halfLen = 8; halfLen < len; halfLen *= 2)
    {
        for (u16 off = 0; off < len; off += halfLen*2)
        {
            for (u16 i = 0; i < halfLen - 1; i++)
            {
                vec[off + i * 2 + 0] = tmp[off + i];
                vec[off + i * 2 + 1] = tmp[off + i + halfLen] + tmp[off + i + halfLen + 1];
            };
        
            vec[off + halfLen*2 - 2] = tmp[off + halfLen - 1];
            vec[off + halfLen*2 - 1] = tmp[off + halfLen*2 - 1];
        };

		FDCT_DATA *t = tmp; tmp = vec; vec = t;
    };
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// DCT type III, unscaled. Algorithm by Byeong Gi Lee, 1984.
// See: https://www.nayuki.io/res/fast-discrete-cosine-transform-algorithms/lee-new-algo-discrete-cosine-transform.pdf

bool FastDctLee_inverseTransform(FDCT_DATA vector[], u16 log2n)
{
	if (log2n < 3 || log2n > FDCT_LOG2N) return false;  // Length is not power of 2

	FDCT_DATA temp[FDCT_N];

	vector[0] /= 2;

	u16 len = 1UL<<log2n;

	inverseTransform(vector, temp, len);

	#ifdef FDCT_INTEGER
		for (u16 i = 0; i < len; i++) vector[i] >>= 2;
	#else
		for (u16 i = 0; i < len; i++) vector[i] /= 4;
	#endif

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void inverseTransform2(FDCT_DATA vector[restrict])
{
	FDCT_DATA x = vector[0];
	FDCT_DATA y = FDCT_MULT(vector[1] * fdct_trig[1]);

	vector[0] = x + y;
	vector[1] = x - y;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void inverseTransform4(FDCT_DATA vector[restrict])
{
	//FDCT_DATA temp3 = vector[1] + vector[3];

	FDCT_DATA x = vector[0];
	FDCT_DATA y = FDCT_MULT(vector[2] * fdct_trig[1]);

	FDCT_DATA temp0 = x + y;
	FDCT_DATA temp1 = x - y;

	y = FDCT_MULT((vector[1] + vector[3]) * fdct_trig[1]);

	FDCT_DATA temp2 = vector[1] + y;
	FDCT_DATA temp3 = vector[1] - y;

	y = FDCT_MULT(temp2 * fdct_trig[2]); // / (cos((i + 0.5) * M_PI / len) * 2);

	vector[0] = temp0 + y;
	vector[3] = temp0 - y;

	y = FDCT_MULT(temp3 * fdct_trig[3]); // / (cos((i + 0.5) * M_PI / len) * 2);

	vector[1] = temp1 + y;
	vector[2] = temp1 - y;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void inverseTransform(FDCT_DATA vector[restrict], FDCT_DATA temp[restrict], u16 len)
{
	if (len == 1) return;

	u16 halfLen = len / 2;

	temp[0] = vector[0];
	temp[halfLen] = vector[1];
	
	for (u16 i = 1; i < halfLen; i++)
	{
		temp[i] = vector[i * 2];
		temp[i + halfLen] = vector[i * 2 - 1] + vector[i * 2 + 1];
	};

	if (halfLen != 4)
	{
		inverseTransform(temp,			vector, halfLen);
		inverseTransform(temp+halfLen,	vector, halfLen);
	}
	else
	{
		inverseTransform4(temp			);
		inverseTransform4(temp+halfLen	);
	};

	FDCT_TRIG *trig = fdct_trig + halfLen;

	for (u16 i = 0; i < halfLen; i++)
	{
		FDCT_DATA x = temp[i];
		FDCT_DATA y = FDCT_MULT(temp[i + halfLen] * trig[i]); // / (cos((i + 0.5) * M_PI / len) * 2);

		vector[i]			= x + y;
		vector[len - 1 - i]	= x - y;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef _MSC_VER

//#undef FDCT_DATA
//#undef FDCT_TRIG
//#undef FDCT_MULT
//#undef FDCT_FLOAT
//#undef FDCT_LOG2N


#define FDCTFLOAT(x) ((FDCTDATA)(x))
#define FDCTMULT(x)  (x)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static FDCTTRIG fdcttrig[FDCT_MAX] = { 0 };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FDCT_WIN_Init()
{
	for (u32 len = 1; len < ArraySize(fdcttrig); len *= 2)
	{
		FDCTTRIG* trig = fdcttrig + len;

		for (u32 i = 0; i < len; i++)
		{
			trig[i] = FDCTFLOAT(0.5f / cos((i + 0.5) * M_PI / (len * 2)));
		};
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void Forward_8(FDCTDATA vector[])
{
	FDCTDATA temp0 = vector[0] + vector[7];
	FDCTDATA temp4 = FDCTMULT((vector[0] - vector[7]) * fdcttrig[4]); // / (cos((i + 0.5) * M_PI / len) * 2);

	FDCTDATA temp1 = vector[1] + vector[6];
	FDCTDATA temp5 = FDCTMULT((vector[1] - vector[6]) * fdcttrig[5]); // / (cos((i + 0.5) * M_PI / len) * 2);

	FDCTDATA temp2 = vector[2] + vector[5];
	FDCTDATA temp6 = FDCTMULT((vector[2] - vector[5]) * fdcttrig[6]); // / (cos((i + 0.5) * M_PI / len) * 2);

	FDCTDATA temp3 = vector[3] + vector[4];
	FDCTDATA temp7 = FDCTMULT((vector[3] - vector[4]) * fdcttrig[7]); // / (cos((i + 0.5) * M_PI / len) * 2);

	FDCTDATA t0 = temp0 + temp3;
	FDCTDATA x = FDCTMULT((temp0 - temp3) * fdcttrig[2]);

	FDCTDATA t1 = temp1 + temp2;
	FDCTDATA y = FDCTMULT((temp1 - temp2) * fdcttrig[3]);

	vector[0] = t0 + t1;								//temp0
	vector[4] = FDCTMULT((t0 - t1) * fdcttrig[1]);	// temp2

	vector[6] = FDCTMULT((x - y) * fdcttrig[1]);
	vector[2] = x + y + vector[6];	// temp1

	t0 = temp4 + temp7;
	x = FDCTMULT((temp4 - temp7) * fdcttrig[2]);

	t1 = temp5 + temp6;
	y = FDCTMULT((temp5 - temp6) * fdcttrig[3]);

	temp4 = t0 + t1;
	temp6 = FDCTMULT((t0 - t1) * fdcttrig[1]);

	t0 = FDCTMULT((x - y) * fdcttrig[1]);
	temp5 = x + y + t0;

	vector[1] = temp4 + temp5;
	vector[3] = temp5 + temp6;
	vector[5] = temp6 + t0;
	vector[7] = t0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Forward_Transform_v3(FDCTDATA vector[], FDCTDATA temp[], u16 len)
{
	//u16 halfLen = len / 2;

	FDCTDATA* vec = temp;
	FDCTDATA* tmp = vector;

	for (u16 halfLen = len / 2; halfLen > 4; halfLen /= 2)
	{
		FDCTTRIG* trig = fdcttrig + halfLen;

		FDCTDATA* t = tmp; tmp = vec; vec = t;

		for (u16 off = 0; off < len; off += halfLen * 2)
		{
			for (u16 i = 0; i < halfLen; i++)
			{
				FDCTDATA x = vec[off + i];
				FDCTDATA y = vec[off + halfLen * 2 - 1 - i];
				tmp[off + i] = x + y;
				tmp[off + i + halfLen] = FDCTMULT((x - y) * trig[i]);
			};
		};

	};

	for (u16 i = 0; i < len; i += 8) Forward_8(tmp + i);

	for (u16 halfLen = 8; halfLen < len; halfLen *= 2)
	{
		for (u16 off = 0; off < len; off += halfLen * 2)
		{
			for (u16 i = 0; i < halfLen - 1; i++)
			{
				vec[off + i * 2 + 0] = tmp[off + i];
				vec[off + i * 2 + 1] = tmp[off + i + halfLen] + tmp[off + i + halfLen + 1];
			};

			vec[off + halfLen * 2 - 2] = tmp[off + halfLen - 1];
			vec[off + halfLen * 2 - 1] = tmp[off + halfLen * 2 - 1];
		};

		FDCTDATA* t = tmp; tmp = vec; vec = t;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool FDCT_Forward(FDCTDATA vector[], FDCTDATA temp[], u16 log2n, float scale)
{
	if (log2n < 3 || log2n > FDCT_LOG2MAX || temp == 0 || vector == 0) return false;  // Length is not power of 2

	u16 len = 1UL << log2n;

	Forward_Transform_v3(vector, temp, len);

	vector[0] /= 2;

	for (u16 i = 0; i < len; i++) vector[i] *= scale;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool PW_FDCT_Forward(const i16* src, FDCTDATA vector[], u16 log2n, float scale)
{
	if (log2n < 3 || log2n > FDCT_LOG2MAX || src == 0 || vector == 0) return false;  // Length is not power of 2

	FDCTDATA temp[FDCT_MAX];

	u16 len = 1UL << log2n;

	for (u16 i = 0; i < len; i++) vector[i] = src[i];

	Forward_Transform_v3(vector, temp, len);

	vector[0] /= 2;

	for (u16 i = 0; i < len; i++) vector[i] *= scale;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void Inverse_4(FDCTDATA vector[])
{
	FDCTDATA x = vector[0];
	FDCTDATA y = FDCTMULT(vector[2] * fdcttrig[1]);

	FDCTDATA temp0 = x + y;
	FDCTDATA temp1 = x - y;

	y = FDCTMULT((vector[1] + vector[3]) * fdcttrig[1]);

	FDCTDATA temp2 = vector[1] + y;
	FDCTDATA temp3 = vector[1] - y;

	y = FDCTMULT(temp2 * fdcttrig[2]); // / (cos((i + 0.5) * M_PI / len) * 2);

	vector[0] = temp0 + y;
	vector[3] = temp0 - y;

	y = FDCTMULT(temp3 * fdcttrig[3]); // / (cos((i + 0.5) * M_PI / len) * 2);

	vector[1] = temp1 + y;
	vector[2] = temp1 - y;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Inverse_Transform(FDCTDATA vector[], FDCTDATA temp[], u16 len)
{
	if (len == 1) return;

	u16 halfLen = len / 2;

	temp[0] = vector[0];
	temp[halfLen] = vector[1];

	for (u16 i = 1; i < halfLen; i++)
	{
		temp[i] = vector[i * 2];
		temp[i + halfLen] = vector[i * 2 - 1] + vector[i * 2 + 1];
	};

	if (halfLen != 4)
	{
		Inverse_Transform(temp, vector, halfLen);
		Inverse_Transform(temp + halfLen, vector, halfLen);
	}
	else
	{
		Inverse_4(temp);
		Inverse_4(temp + halfLen);
	};

	FDCTTRIG* trig = fdcttrig + halfLen;

	for (u16 i = 0; i < halfLen; i++)
	{
		FDCTDATA x = temp[i];
		FDCTDATA y = FDCTMULT(temp[i + halfLen] * trig[i]); // / (cos((i + 0.5) * M_PI / len) * 2);

		vector[i] = x + y;
		vector[len - 1 - i] = x - y;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool FDCT_Inverse(FDCTDATA *vect, FDCTDATA *temp, u16 log2n, float scale)
{
	if (log2n < 3 || log2n > FDCT_LOG2MAX || vect == 0 || temp == 0) return false;  // Length is not power of 2

	u16 len = 1UL << log2n;

	Inverse_Transform(vect, temp, len);

	for (u16 i = 0; i < len; i++) vect[i] *= scale;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool PW_FDCT_Inverse(const FDCTDATA src[], i16* dst, u16 log2n, float scale)
{
	if (log2n < 3 || log2n > FDCT_LOG2MAX || src == 0 || dst == 0) return false;  // Length is not power of 2

	FDCTDATA vector[FDCT_MAX];
	FDCTDATA temp[FDCT_MAX];

	u16 len = 1UL << log2n;

	for (u16 i = 0; i < len; i++) vector[i] = src[i];

	Inverse_Transform(vector, temp, len);

	for (u16 i = 0; i < len; i++) dst[i] = (i16)(LIM(vector[i] * scale, -32767, 32767));

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



#endif // #ifdef _MSC_VER

#endif // FDCT_IMP_H__08_11_2023__15_16
