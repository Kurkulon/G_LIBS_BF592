#ifndef BF592_H__03_03_2014__11_41
#define BF592_H__03_03_2014__11_41

#ifndef CORETYPE_BF592
//#error  Must #include "core.h"
#endif 

#include <bfrom.h>
#include <sys\exception.h>
#include <cdefBF592-A.h>
#include <sysreg.h>

#include "types.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern void InitIVG(u32 IVG, u32 PID, void (*EVT)());

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void ResetWDT()		{ *pWDOG_STAT = 0;		}
inline void DisableWDT()	{ *pWDOG_CTL = WDDIS;	}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u64 GetCycles64()
{
	u64 res;

	__asm volatile ("CLI r0;       \n"  
                    "r2 = CYCLES;  \n"  
                    "r1 = CYCLES2; \n"  
                    "STI r0;       \n"  
                    "[%0]   = r2;  \n"  
                    "[%0+4] = r1;  \n"  
                    : : "p" (&res) 
                    : "r0", "r1", "r2" ); 

	return res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma always_inline
inline u32 GetCycles32()
{
	//u32 res;

	//__asm volatile ("%0 = CYCLES;  \n"	: "=d" (res)	:	:	); 

	return sysreg_read(reg_CYCLES); 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define MS2RT(x) MS2CCLK(x)
#define US2RT(x) US2CCLK(x)

//inline u32 GetRTT() { extern u32 mmsec; return mmsec; }

struct RTM32
{
	u32 pt;

	RTM32() : pt(0) {}
	bool Check(u32 v) { u32 t = GetCycles32(); if ((t - pt) >= v) { pt = t; return true; } else { return false; }; }
	bool Timeout(u32 v) { return (u32)(GetCycles32() - pt) >= v; }
	void Reset() { pt = GetCycles32(); }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32
#define MK_PTR(n,p)  T_HW::S_##n * const n = ((T_HW::S_##n*)(p))
#else
extern byte core_sys_array[0x100000]; 
#define MK_PTR(n,p)  T_HW::S_##n * const n = ((T_HW::S_##n*)(core_sys_array-0x40000000+p))
#endif


#define MKPID(n,i) n##_M=(1UL<<(i&31)), n##_I=i

namespace T_HW
{
	typedef volatile u16 BF_R16;// Hardware register definition
	typedef volatile u32 BF_REG;// Hardware register definition
	typedef volatile void * BF_PTR;// Hardware register definition

	typedef void(*BF_IHP)();	// Interrupt handler pointer

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_SMC
	{
		BF_REG	NFC_CFG;	
		BF_REG	NFC_CTRL;	
		BF_REG	NFC_SR;	
		BF_REG	NFC_IER;	
		BF_REG	NFC_IDR;	
		BF_REG	NFC_IMR;	
		BF_REG	NFC_ADDR;	
		BF_REG	NFC_BANK;	
		BF_REG	ECC_CTRL;	
		BF_REG	ECC_MD;	
		BF_REG	ECC_SR1;	
		BF_REG	ECC_PR0;	
		BF_REG	ECC_PR1;	
		BF_REG	ECC_SR2;	
		BF_REG	ECC_PR2;	
		BF_REG	ECC_PR3;	
		BF_REG	ECC_PR4;	
		BF_REG	ECC_PR5;	
		BF_REG	ECC_PR6;	
		BF_REG	ECC_PR7;	
		BF_REG	ECC_PR8;	
		BF_REG	ECC_PR9;	
		BF_REG	ECC_PR10;	
		BF_REG	ECC_PR11;	
		BF_REG	ECC_PR12;	
		BF_REG	ECC_PR13;	
		BF_REG	ECC_PR14;	
		BF_REG	ECC_PR15;	

		struct S_CSR { BF_REG	SETUP, PULSE, CYCLE, TIMINGS, MODE;	} CSR[8];

		BF_REG	OCMS;	
		BF_REG	KEY1;	
		BF_REG	KEY2;	
		BF_REG	WPCR;	
		BF_REG	WPSR;	
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}


namespace HW
{
	//namespace PID
	//{
	//	enum {	MKPID(SUPC, 0),		MKPID(RSTC, 1),		MKPID(RTC, 2),		MKPID(RTT, 3),		MKPID(WDT, 4),		MKPID(PMC, 5),		MKPID(EEFC0, 6),	MKPID(EEFC1, 7),
	//			MKPID(UART, 8),		MKPID(SMC, 9),		MKPID(SDRAM, 10),	MKPID(PIOA, 11),	MKPID(PIOB, 12),	MKPID(PIOC, 13),	MKPID(PIOD, 14),	MKPID(PIOE, 15),
	//			MKPID(PIOF, 16),	MKPID(USART0, 17),	MKPID(USART1, 18),	MKPID(USART2, 19),	MKPID(USART3, 20),	MKPID(HSMCI, 21),	MKPID(TWI0, 22),	MKPID(TWI1, 23), 
	//			MKPID(SPI0, 24),	MKPID(SPI1, 25), 	MKPID(SSC, 26),		MKPID(TC0, 27),		MKPID(TC1, 28),		MKPID(TC2, 29),		MKPID(TC3, 30),		MKPID(TC4, 31), 
	//			MKPID(TC5, 32),		MKPID(TC6, 33),		MKPID(TC7, 34),		MKPID(TC8, 35), 	MKPID(PWM, 36),		MKPID(ADC, 37),		MKPID(DACC, 38),	MKPID(DMAC, 39), 
	//			MKPID(UOTGHS, 40),	MKPID(TRNG, 41),	MKPID(EMAC, 42),	MKPID(CAN0, 43),	MKPID(CAN1, 44) };
	//};

	//MK_PTR(SMC, 	0x400E0000);

	//MK_PTR(MATRIX,	0x400E0400);
	//MK_PTR(PMC,		0x400E0600);
	//MK_PTR(UART,	0x400E0800);

	//MK_PTR(EFC0,	0x400E0A00);
	//MK_PTR(EFC1,	0x400E0C00);
	//MK_PTR(PIOA,	0x400E0E00);
	//MK_PTR(PIOB,	0x400E1000);
	//MK_PTR(PIOC,	0x400E1200);
	//MK_PTR(PIOD,	0x400E1400);

	//MK_PTR(RSTC,	0X400E1A00);
	//MK_PTR(SUPC,	0X400E1A10);
	//MK_PTR(RTT,		0X400E1A30);
	//MK_PTR(WDT,		0X400E1A50);
	//MK_PTR(RTC,		0X400E1A60);

	//MK_PTR(SPI,		0X40008000);
	//MK_PTR(SPI0,	0X40008000);
	//MK_PTR(SPI1,	0X4000C000);

	//MK_PTR(TC0,		0X40080000);
	//MK_PTR(TC1,		0X40084000);
	//MK_PTR(TC2,		0X40088000);

	//MK_PTR(PWM,		0X40094000);
	//MK_PTR(USART0,	0x40098000);
	//MK_PTR(USART1,	0x4009C000);
	//MK_PTR(USART2,	0x400A0000);
	//MK_PTR(USART3,	0x400A4000);

	//MK_PTR(ADC,		0X400C0000);

	//MK_PTR(DMAC,	0X400C4000);


//	MK_PTR(UDP,		0x400A4000);



	inline void ResetWDT()		{ *pWDOG_STAT = 0; }
	inline void DisableWDT()	{ *pWDOG_CTL = WDDIS; }


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


} // namespace HW

//extern T_HW::BF_IHP VectorTableInt[16];
//extern T_HW::BF_IHP VectorTableExt[45];

#define PID_Watchdog_Timer				(31)
#define PID_DMA_14_15_Mem_DMA_Stream_1	(30)
#define PID_DMA_12_13_Mem_DMA_Stream_0	(29)
#define PID_TWI							(24)
#define PID_Port_G_Interrupt_B			(23)
#define PID_Port_G_Interrupt_A			(22)
#define PID_GP_Timer_2					(21)
#define PID_GP_Timer_1					(20)
#define PID_GP_Timer_0					(19)
#define PID_Port_F_Interrupt_B			(18)
#define PID_Port_F_Interrupt_A			(17)
#define PID_DMA8_UART0_TX				(16)
#define PID_DMA7_UART0_RX				(15)
#define PID_DMA6_SPI1_RX_TX				(14)
#define PID_DMA5_SPI0_RX_TX				(13)
#define PID_DMA4_SPORT1_TX				(12)
#define PID_DMA3_SPORT1_RX				(11)
#define PID_DMA2_SPORT0_TX				(10)
#define PID_DMA1_SPORT0_RX				(9)
#define PID_DMA0_PPI					(8)
#define PID_UART0_Status				(7)
#define PID_SPI1_Status					(6) 
#define PID_SPI0_Status					(5) 
#define PID_SPORT1_Status				(4) 
#define PID_SPORT0_Status				(3) 
#define PID_PPI_Status					(2) 
#define PID_DMA_Error_generic			(1) 
#define PID_PLL_Wakeup					(0) 


#undef MK_PTR
#undef MKPID

#endif // BF592_H__03_03_2014__11_41
