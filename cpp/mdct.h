/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: modified discrete cosine transform prototypes
 last mod: $Id: mdct.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#ifndef MDCT_H__05_02_2024__10_58
#define MDCT_H__05_02_2024__10_58

#pragma once

#include "types.h"

//#define MDCT_INTEGERIZED //  <- be warned there could be some hurt left here

#ifdef MDCT_INTEGERIZED

#define MDCT_DATA		i32
#define MDCT_TRIG		i16
#define MDCT_REG		register i32
#define MDCT_TRIGBITS	14

#else

#define MDCT_DATA	float
#define MDCT_TRIG	float
#define MDCT_REG	float

#endif

#define MDCT_BITREV			u16

struct MDCT_LookUp
{
	u16			n;
	u16			log2n;
  
	MDCT_TRIG	*trig;		// len = n
	MDCT_TRIG	*win;		// len = n/2
	MDCT_BITREV	*bitrev;	// len = n/4;
	MDCT_DATA	*work;		// len = n, used mdct_forward
	MDCT_DATA	*temp1;		// len = n
	MDCT_DATA	*temp2;		// len = n

	MDCT_TRIG	scale;
};	

extern void mdct_init(MDCT_LookUp* lookup, int log2n, MDCT_BITREV* bitrev, MDCT_TRIG* T, MDCT_TRIG* win);
//extern void mdct_clear(MDCT_LookUp *l);
extern void mdct_window(MDCT_LookUp* lookup, i16* src, MDCT_DATA* dst);
extern void mdct_forward(MDCT_LookUp *init, MDCT_DATA *in, MDCT_DATA *out);
extern void mdct_backward(MDCT_LookUp *init, MDCT_DATA *in, MDCT_DATA *out);

#endif // MDCT_H__05_02_2024__10_58













