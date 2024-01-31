#ifndef TYPES_H__15_05_2009__14_37
#define TYPES_H__15_05_2009__14_37

typedef unsigned char byte, u8;
typedef unsigned short int word, u16;
typedef unsigned long int dword, u32;
typedef signed char i8;
typedef signed short int i16;
typedef signed long int i32;
typedef signed long long int i64;
typedef unsigned long long int u64;

#ifdef _ADI_COMPILER

#include <ccblkfn.h>

#ifdef __DEBUG
#define __breakpoint() asm("EMUEXCPT;")
#else
#define __breakpoint()
#endif

inline u16 ReverseWord(u16 v) { return byteswap2(v); }
inline u32 ReverseDword(u32 v) { return byteswap4(v); }

#endif // _ADI_COMPILER

#ifdef _MSC_VER //WIN32

	#define WINDOWS_IGNORE_PACKING_MISMATCH

	#include <intrin.h>

	#define __packed __declspec(align(1))
	#define __softfp /**/
	#define __irq __declspec(naked)
	#define __align(v)
	#define __attribute__(v)
	#define __func__ __FUNCTION__
	#define restrict /**/

	__forceinline void __breakpoint(int v) { __debugbreak(); }
	__forceinline void __disable_irq() {}
	__forceinline void __enable_irq() {}
	//inline void __nop() {}

	#if _MSC_VER > 1500
	#pragma comment(lib, "legacy_stdio_definitions.lib")
	#endif

#else //#ifdef _MSC_VER

	#ifndef NAN
	static const dword __NAN_dword = 0xFFFFFFFF;
	#define NAN (*((const float*)(&__NAN_dword)))
	#endif

#endif // #ifdef _MSC_VER 


#define ArraySize(x) (sizeof(x)/sizeof(x[0]))

inline float ABS(float v) { *((u32*)&v) &= 0x7FFFFFFF; return v; }

#define LIM(v, min, max)	(((v) < (min)) ? (min) : (((v) > (max)) ? (max) : (v)))
#define MIN(a, b)			(((a) < (b)) ? (a) : (b))
#define MAX(a, b)			(((a) >= (b)) ? (a) : (b))


inline bool fIsValid(float v) { return (((u16*)&v)[2] & 0x7F80) != 0x7F80; }
inline bool dIsValid(float v) { return (((u32*)&v)[2] & 0x7FF0) != 0x7FF0; }

#define GD(adr, t, i) (*(((t*)adr)+i))
#define GB(adr,i) (*(((byte*)adr)+i))


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
union DataCRC
{
	word	w;
	byte	b[2];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

union U16u 
{
	word w; byte b[2]; 

	U16u(word v) {w = v;}
	operator word() {return w;}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

union U32u 
{ 
	dword d; word w[2]; byte b[4]; 

	U32u(dword v) {d = v;}
	U32u() {d = 0;}
	operator dword() {return d;}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

union F32u
{ 
	float f; dword d; word w[2]; byte b[4]; 

	F32u (float v) {f = v;}
	operator float() {return f;}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

union DataPointer
{
	void	*v;
	byte	*b;
	word 	*w;
	dword	*d;
	float	*f;

	DataPointer(void *p) { v = p; } 

	void operator=(void *p) { v = p; } 

#ifdef _ADI_COMPILER
	void WW(word a) { misaligned_store16(v, a); }
	word RW() { return misaligned_load16(v); }
#endif

} ;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

union ConstDataPointer
{
	const void		*v;
	const byte		*b;
	const word		*w;
	const dword	*d;
	const float	*f;

	ConstDataPointer(const void *p) { v = p; } 

	void operator=(const void *p) { v = p; } 
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#endif // TYPES_H__15_05_2009__14_37
