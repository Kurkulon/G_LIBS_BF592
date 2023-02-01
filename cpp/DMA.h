#ifndef DMA_H__26_05_2022__18_11
#define DMA_H__26_05_2022__18_11

#include "types.h"
#include "bf592.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class DMA_CH
{
protected:

	T_HW::S_DMACH*				const _dmach;

	//T_HW::DMADESC*				const _dmadsc;
	//T_HW::DMADESC*				const _dmawrb;

	//const u32					_act_mask;
	
	const byte					_chnum;

	T_HW::DMADSC_LLM			_dsc1;
	T_HW::DMADSC_LLM			_dsc2;

public:

	void Write(const volatile void *src, u16 len, u16 ctrl);
	void Write(const volatile void *src1, u16 len1, const volatile void *src2, u16 len2, u16 ctrl);
	void Read(volatile void *dst, u16 len, u16 ctrl);
	
	DMA_CH(byte chnum) : _dmach(&HW::DMA->CH[chnum]), _chnum(chnum) { }

	//void Enable() {  }
	
	inline void Disable() { _dmach->CONFIG = 0; }
	inline bool CheckComplete() { return _dmach->IRQ_STATUS & DMA_DONE; }
	inline u32 GetBytesLeft()	{ return _dmach->CURR_X_COUNT; }
	inline u32 GetBytesReady()	{ return _dmach->X_COUNT-_dmach->CURR_X_COUNT; }

	inline void Write8(const volatile void *src, u16 len) { Write(src, len, WDSIZE_8); }
	inline void Write16(const volatile void *src, u16 len) { Write(src, len, WDSIZE_16); }
	inline void Write32(const volatile void *src, u16 len) { Write(src, len, WDSIZE_32); }

	inline void Read8(volatile void *dst, u16 len) { Read(dst, len, WDSIZE_8); }
	inline void Read16(volatile void *dst, u16 len) { Read(dst, len, WDSIZE_16); }
	inline void Read32(volatile void *dst, u16 len) { Read(dst, len, WDSIZE_32); }

	inline void Write8(const volatile void *src1, u16 len1, const volatile void *src2, u16 len2) { Write(src1, len1, src2, len2, WDSIZE_8); }
	inline void Write16(const volatile void *src1, u16 len1, const volatile void *src2, u16 len2) { Write(src1, len1, src2, len2, WDSIZE_16); }
	inline void Write32(const volatile void *src1, u16 len1, const volatile void *src2, u16 len2) { Write(src1, len1, src2, len2, WDSIZE_32); }

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_SAME53

extern DMA_CH		DMA_CH0	;
extern DMA_CH		DMA_CH1	;
extern DMA_CH		DMA_CH2	;
extern DMA_CH		DMA_CH3	;
extern DMA_CH		DMA_CH4	;
extern DMA_CH		DMA_CH5	;
extern DMA_CH		DMA_CH6	;
extern DMA_CH		DMA_CH7	;
extern DMA_CH		DMA_CH8	;
extern DMA_CH		DMA_CH9	;
extern DMA_CH		DMA_CH10;
extern DMA_CH		DMA_CH11;
extern DMA_CH		DMA_CH12;
extern DMA_CH		DMA_CH13;
extern DMA_CH		DMA_CH14;
extern DMA_CH		DMA_CH15;
extern DMA_CH		DMA_CH16;
extern DMA_CH		DMA_CH17;
extern DMA_CH		DMA_CH18;
extern DMA_CH		DMA_CH19;
extern DMA_CH		DMA_CH20;
extern DMA_CH		DMA_CH21;
extern DMA_CH		DMA_CH22;
extern DMA_CH		DMA_CH23;
extern DMA_CH		DMA_CH24;
extern DMA_CH		DMA_CH25;
extern DMA_CH		DMA_CH26;
extern DMA_CH		DMA_CH27;
extern DMA_CH		DMA_CH28;
extern DMA_CH		DMA_CH29;
extern DMA_CH		DMA_CH30;
extern DMA_CH		DMA_CH31;

#elif defined(CPU_XMC48)

extern DMA_CH		DMA_CH0;
extern DMA_CH		DMA_CH1;
extern DMA_CH		DMA_CH2;
extern DMA_CH		DMA_CH3;
extern DMA_CH		DMA_CH4;
extern DMA_CH		DMA_CH5;
extern DMA_CH		DMA_CH6;
extern DMA_CH		DMA_CH7;
extern DMA_CH		DMA_CH8;
extern DMA_CH		DMA_CH9;
extern DMA_CH		DMA_CH10;
extern DMA_CH		DMA_CH11;

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // DMA_H__26_05_2022__18_11
