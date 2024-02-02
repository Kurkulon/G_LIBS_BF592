#ifndef FDCT_H__08_11_2023__15_15
#define FDCT_H__08_11_2023__15_15

/* 
 * Fast discrete cosine transform algorithms (C)
 * 
 * Copyright (c) 2018 Project Nayuki. (MIT License)
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

#pragma once

#include "types.h"

#ifndef FDCT_FLOAT

#ifndef FDCT_DATA
#define FDCT_DATA	i32
#endif

#ifndef FDCT_TRIG
#define FDCT_TRIG	i16
#endif

#else

#define FDCT_DATA float;
#define FDCT_TRIG float;

#endif 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct PackDCT
{
	byte	len;
	byte	scale;
	byte	data[16];
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern bool FastDctLee_transform(FDCT_DATA vector[], u16 log2n);
extern bool FastDctLee_inverseTransform(FDCT_DATA vector[], u16 log2n);

extern void FDCT_Init();

#ifdef _MSC_VER

#define FDCT_LOG2MAX 12
#define FDCT_MAX (1UL<<FDCT_LOG2MAX)

typedef float FDCTDATA;
typedef float FDCTTRIG;

extern void FDCT_WIN_Init();
extern bool FDCT_Forward(FDCTDATA vector[], FDCTDATA temp[], u16 log2n, float scale);
extern bool FDCT_Inverse(FDCTDATA* vector, FDCTDATA* temp, u16 log2n, float scale);
extern bool PW_FDCT_Inverse(const FDCTDATA src[], i16* dst, u16 log2n, float scale);
extern bool PW_FDCT_Forward(const i16* src, FDCTDATA vector[], u16 log2n, float scale);

#endif


#endif // FDCT_H__08_11_2023__15_15

