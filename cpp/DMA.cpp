#include "DMA.h"

#ifdef CPU_SAME53

	__align(16) T_HW::DMADESC DmaTable[32];
	__align(16) T_HW::DMADESC DmaWRB[32];
	
#elif defined(CPU_XMC48)

#elif defined(CPU_LPC824)

	__align(512) T_HW::DMADESC _DmaTable[18] = {0};

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void DMA_CH::Write(const volatile void *src, u16 len, u16 ctrl)
{
	_dmach->CONFIG = 0;
	_dmach->START_ADDR = (void*)src;
	_dmach->X_COUNT = len;
	_dmach->X_MODIFY = 1UL<<((ctrl>>2)&3);

	_dmach->IRQ_STATUS = DMA_DONE;
	_dmach->CONFIG = FLOW_STOP|DI_EN|(ctrl&(WDSIZE_16|WDSIZE_32))|SYNC|DMAEN;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void DMA_CH::Write(const volatile void *src1, u16 len1, const volatile void *src2, u16 len2, u16 ctrl)
{
	_dmach->CONFIG = 0;

	ctrl = (ctrl&(WDSIZE_16|WDSIZE_32))|SYNC|DMAEN;

	_dsc1.SA = (void*)src1;
	_dsc1.XCNT = len1;
	_dsc1.XMOD =  1UL<<((ctrl>>2)&3);

	if (src2 != 0 && len2 != 0)
	{
		_dsc1.NDP = &_dsc2;
		_dsc1.DMACFG = FLOW_LARGE|NDSIZE_9|ctrl;

		_dsc2.SA = (void*)src2;
		_dsc2.XCNT = len2;
		_dsc2.XMOD =  1UL<<((ctrl>>2)&3);
		_dsc2.DMACFG = FLOW_STOP|DI_EN|ctrl;
	}
	else
	{
		_dsc1.DMACFG = FLOW_STOP|DI_EN|ctrl;
	};

	_dmach->IRQ_STATUS = DMA_DONE;

	_dmach->NEXT_DESC_PTR = &_dsc1;
	_dmach->CONFIG = FLOW_LARGE|NDSIZE_9|DMAEN;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void DMA_CH::Read(volatile void *dst, u16 len, u16 ctrl)
{
	_dmach->CONFIG = 0;
	_dmach->START_ADDR = (void*)dst;
	_dmach->X_COUNT = len;
	_dmach->X_MODIFY = 1UL<<((ctrl>>2)&3);

	_dmach->IRQ_STATUS = DMA_DONE;
	_dmach->CONFIG = FLOW_STOP|DI_EN|WNR|(ctrl&(WDSIZE_16|WDSIZE_32))|SYNC|DMAEN;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

