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

#pragma always_inline
inline void SIC_EnableIRQ(byte pid) { *pSIC_IMASK |= 1UL<<pid; }
inline void SIC_DisableIRQ(byte pid) { *pSIC_IMASK &= ~(1UL<<pid); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma always_inline
inline void ResetWDT()		{ *pWDOG_STAT = 0;		}
#pragma always_inline
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
	typedef volatile u16	BF_R16;		// Hardware register definition
	typedef volatile u32	BF_R32;		// Hardware register definition
	typedef volatile void	*BF_PTR;	// Hardware register definition


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_SPI
	{
		BF_R16	Ctl;					//	SPI Control Register	
		BF_R16			z__Reserved1;
		BF_R16	Flg;					//	SPI Flag register								
		BF_R16			z__Reserved2;
		BF_R16	Stat;					//	SPI Status register							
		BF_R16			z__Reserved3;
		BF_R16	TDBR;					//	SPI Transmit Data Buffer Register				
		BF_R16			z__Reserved4;
		BF_R16	RDBR;					//	SPI Receive Data Buffer Register				
		BF_R16			z__Reserved5;
		BF_R16	Baud;					//	SPI Baud rate Register							
		BF_R16			z__Reserved6;
		BF_R16	Shadow;					//	SPI_RDBR Shadow Register						
	};

	typedef S_SPI S_SPI0, S_SPI1;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_TIMERx
	{
		BF_R16	Config;				//	Timer x Configuration Register					
		BF_R16			z__Reserved1;
		BF_R32	Counter;			//	Timer x Counter Register						
		BF_R32	Period;				//	Timer x Period Register							
		BF_R32	Width;				//	Timer x Width Register		
	};

	typedef S_TIMERx S_TIMER0, S_TIMER1, S_TIMER2;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_TIMER
	{
		BF_R16	Enable;					//	Timer Enable Register	
		BF_R16			z__Reserved1;
		BF_R16	Disable;				//	Timer Disable Register	
		BF_R16			z__Reserved2;
		BF_R16	Status;					//	Timer Status Register	
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_PIO
	{
		BF_R16	PinState;			//	Port I/O Pin State Specify Register				
		BF_R16			z__Reserved1;
		BF_R16	Clear;				//	Port I/O Peripheral Interrupt Clear Register	
		BF_R16			z__Reserved2;
		BF_R16	Set;				//	Port I/O Peripheral Interrupt Set Register		
		BF_R16			z__Reserved3;
		BF_R16	Toggle;				//	Port I/O Pin State Toggle Register				
		BF_R16			z__Reserved4;
		BF_R16	MaskA;				//	Port I/O Mask State Specify Interrupt A Register
		BF_R16			z__Reserved5;
		BF_R16	MaskA_Clr;			//	Port I/O Mask Disable Interrupt A Register		
		BF_R16			z__Reserved6;
		BF_R16	MaskA_Set;			//	Port I/O Mask Enable Interrupt A Register		
		BF_R16			z__Reserved7;
		BF_R16	MaskA_Tgl;			//	Port I/O Mask Toggle Enable Interrupt A Register
		BF_R16			z__Reserved8;
		BF_R16	MaskB;				//	Port I/O Mask State Specify Interrupt B Register
		BF_R16			z__Reserved9;
		BF_R16	MaskB_Clr;			//	Port I/O Mask Disable Interrupt B Register		
		BF_R16			z__Reserved10;
		BF_R16	MaskB_Set;			//	Port I/O Mask Enable Interrupt B Register		
		BF_R16			z__Reserved11;
		BF_R16	MaskB_Tgl;			//	Port I/O Mask Toggle Enable Interrupt B Register
		BF_R16			z__Reserved12;
		BF_R16	Dir;				//	Port I/O Direction Register						
		BF_R16			z__Reserved13;
		BF_R16	Polar;				//	Port I/O Source Polarity Register				
		BF_R16			z__Reserved14;
		BF_R16	Edge;				//	Port I/O Source Sensitivity Register			
		BF_R16			z__Reserved51;
		BF_R16	Both;				//	Port I/O Set on BOTH Edges Register				
		BF_R16			z__Reserved16;
		BF_R16	Inen;				//	Port I/O Input Enable Register 		

		inline void 	SET(u32 m) 			{ Set = m; }
		inline void 	CLR(u32 m) 			{ Clear = m; }
		inline void 	NOT(u32 m) 			{ Toggle = m; }
		inline void 	WBIT(u32 m, bool c) { if (c) SET(m); else CLR(m); }
		inline void 	BSET(u16 b) 		{ Set = 1UL<< b; }
		inline void 	BCLR(u16 b) 		{ Clear = 1UL << b; }
		inline void 	BTGL(u16 b) 		{ Toggle = 1UL << b; }

		inline bool 	TBSET(u16 b) 		{ return PinState & (1<<b); }
		inline bool 	TBCLR(u16 b) 		{ return (PinState & (1<<b)) == 0; }
	};

	typedef S_PIO S_PIOF, S_PIOG;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_PORT
	{
		BF_R16	FER;						//	Port x Function Enable Register (Alternate/Flag*)
		BF_R16			z__Reserved1;
		BF_R16	MUX;            			//	Port x mux control 								
		BF_R16			z__Reserved3;
		BF_R16	PADCTL;        				//	Port x pad control 	
	};

	typedef S_PORT S_PORTF, S_PORTG;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_SPORT
	{
		BF_R16	TCR1;						//	SPORT Transmit Configuration 1 Register			
		BF_R16			z__Reserved1;
		BF_R16	TCR2;						//	SPORT Transmit Configuration 2 Register			
		BF_R16			z__Reserved2;
		BF_R16	TCLKDIV;					//	SPORT Transmit Clock Divider					
		BF_R16			z__Reserved3;
		BF_R16	TFSDIV;						//	SPORT Transmit Frame Sync Divider				
		BF_R16			z__Reserved4;
		BF_R32	TX;							//	SPORT TX Data Register							
		BF_R32			z__Reserved5;
		BF_R32	RX;							//	SPORT RX Data Register							
		BF_R16	RCR1;						//	SPORT Transmit Configuration 1 Register			
		BF_R16			z__Reserved6;
		BF_R16	RCR2;						//	SPORT Transmit Configuration 2 Register			
		BF_R16			z__Reserved7;
		BF_R16	RCLKDIV;					//	SPORT Receive Clock Divider						
		BF_R16			z__Reserved8;
		BF_R16	RFSDIV;						//	SPORT Receive Frame Sync Divider				
		BF_R16			z__Reserved9;
		BF_R16	STAT;						//	SPORT Status Register							
		BF_R16			z__Reserved10;
		BF_R16	CHNL;						//	SPORT Current Channel Register					
		BF_R16			z__Reserved11;
		BF_R16	MCMC1;						//	SPORT Multi-Channel Configuration Register 1	
		BF_R16			z__Reserved12;
		BF_R16	MCMC2;						//	SPORT Multi-Channel Configuration Register 2	
		BF_R16			z__Reserved13;
		BF_R32	MTCS0;						//	SPORT Multi-Channel Transmit Select Register 0	
		BF_R32	MTCS1;						//	SPORT Multi-Channel Transmit Select Register 1	
		BF_R32	MTCS2;						//	SPORT Multi-Channel Transmit Select Register 2	
		BF_R32	MTCS3;						//	SPORT Multi-Channel Transmit Select Register 3	
		BF_R32	MRCS0;						//	SPORT Multi-Channel Receive Select Register 0	
		BF_R32	MRCS1;						//	SPORT Multi-Channel Receive Select Register 1	
		BF_R32	MRCS2;						//	SPORT Multi-Channel Receive Select Register 2	
		BF_R32	MRCS3;						//	SPORT Multi-Channel Receive Select Register 3	
	};

	typedef S_SPORT S_SPORT0, S_SPORT1;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	struct DMADSC_AM
	{
		union { u32 SA;	struct { u16 SAL; u16 SAH; }; };
		u16 DMACFG;
		u16 XCNT;
		u16 XMOD;
		u16 YCNT;
		u16 YMOD;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	struct DMADSC_SLM
	{
		u16 NDPL;
		u16 SAL; 
		u16 SAH; 
		u16 DMACFG;
		u16 XCNT;
		u16 XMOD;
		u16 YCNT;
		u16 YMOD;

		inline void SA(u32 v) { SAL = v; SAH = v>>16; }
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct DMADSC_LLM
	{
		union { void* NDP; struct { u16 NDPL; u16 NDPH; }; };
		union { void* SA;	struct { u16 SAL; u16 SAH; }; };
		u16 DMACFG;
		u16 XCNT;
		u16 XMOD;
		u16 YCNT;
		u16 YMOD;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	struct S_DMACH
	{
		BF_PTR	NEXT_DESC_PTR;									//	DMA Channel x Next Descriptor Pointer Register		
		BF_PTR	START_ADDR;										//	DMA Channel x Start Address Register				
		BF_R16	CONFIG;											//	DMA Channel x Configuration Register				
								BF_R16	z__Reserved1[3];							
		BF_R16	X_COUNT;										//	DMA Channel x X Count Register						
								BF_R16	z__Reserved2;							
		BF_R16	X_MODIFY;										//	DMA Channel x X Modify Register						
								BF_R16	z__Reserved3;									
		BF_R16	Y_COUNT;										//	DMA Channel x Y Count Register						
								BF_R16	z__Reserved4;									
		BF_R16	Y_MODIFY;										//	DMA Channel x Y Modify Register						
								BF_R16	z__Reserved5;									
		BF_PTR	CURR_DESC_PTR;									//	DMA Channel x Current Descriptor Pointer Register	
		BF_PTR	CURR_ADDR;										//	DMA Channel x Current Address Register				
		BF_R16	IRQ_STATUS;										//	DMA Channel x Interrupt/Status Register				
								BF_R16	z__Reserved6;									
		BF_R16	PERIPHERAL_MAP;									//	DMA Channel x Peripheral Map Register				
								BF_R16	z__Reserved7;									
		BF_R16	CURR_X_COUNT;									//	DMA Channel x Current X Count Register				
								BF_R16	z__Reserved8[3];									
		BF_R16	CURR_Y_COUNT;									//	DMA Channel x Current Y Count Register				
								BF_R16	z__Reserved9[3];
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_DMA
	{
		BF_R16	TC_PER;						// 0xFFC00B0C	/* Traffic Control Periods Register						
							BF_R16	z__Reserved1;
		BF_R16	TC_CNT;						// 0xFFC00B10	/* Traffic Control Current Counts Register				
							BF_R16	z__Reserved2;
							BF_R32	z__Reserved3[59];
		S_DMACH	CH[9];
							BF_R32	z__Reserved4[49];
		S_DMACH	 MDMA_D0;
		S_DMACH	 MDMA_S0;
		S_DMACH	 MDMA_D1;
		S_DMACH	 MDMA_S1;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_PPI
	{
		BF_R32	Control;			//	PPI Control Register		
		BF_R32	Status;				//	PPI Status Register			
		BF_R32	Count;				//	PPI Transfer Count Register	
		BF_R32	Delay;				//	PPI Delay Count Register	
		BF_R32	Frame;				//	PPI Frame Length Register	
	};
	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_TWI
	{
		BF_R32	CLKDIV;				//	Serial Clock Divider Register			
		BF_R32	CONTROL;			//	TWI Control Register						
		BF_R32	SLAVE_CTL;			//	Slave Mode Control Register				
		BF_R32	SLAVE_STAT;			//	Slave Mode Status Register				
		BF_R32	SLAVE_ADDR;			//	Slave Mode Address Register				
		BF_R32	MASTER_CTL;			//	Master Mode Control Register				
		BF_R32	MASTER_STAT;		//	Master Mode Status Register				
		BF_R32	MASTER_ADDR;		//	Master Mode Address Register				
		BF_R32	INT_STAT;			//	TWI Interrupt Status Register			
		BF_R32	INT_MASK;			//	TWI Master Interrupt Mask Register		
		BF_R32	FIFO_CTL;			//	FIFO Control Register					
		BF_R32	FIFO_STAT;			//	FIFO Status Register						
		BF_R32	XMT_DATA8;			//	FIFO Transmit Data Single Byte Register	
		BF_R32	XMT_DATA16;			//	FIFO Transmit Data Double Byte Register	
		BF_R32	RCV_DATA8;			//	FIFO Receive Data Single Byte Register	
		BF_R32	RCV_DATA16;			//	FIFO Receive Data Double Byte Register	
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

	MK_PTR(SPI0,	SPI0_CTL);
	MK_PTR(SPI1,	SPI1_CTL);

	MK_PTR(TIMER0, 	TIMER0_CONFIG);
	MK_PTR(TIMER1, 	TIMER1_CONFIG);
	MK_PTR(TIMER2, 	TIMER2_CONFIG);

	MK_PTR(TIMER,	TIMER_ENABLE);

	MK_PTR(PIOF,	PORTFIO);
	MK_PTR(PIOG,	PORTGIO);

	MK_PTR(PORTF,	PORTF_FER);
	MK_PTR(PORTG,	PORTG_FER);

	MK_PTR(SPORT0,	SPORT0_TCR1);
	MK_PTR(SPORT1,	SPORT1_TCR1);

	MK_PTR(DMA,		DMA_TC_PER);

	MK_PTR(PPI,		PPI_CONTROL);

	MK_PTR(TWI,		TWI_CLKDIV);

	//MK_PTR(SUPC,	0X400E1A10);
	//MK_PTR(RTT,		0X400E1A30);
	//MK_PTR(WDT,		0X400E1A50);
	//MK_PTR(RTC,		0X400E1A60);

	//MK_PTR(SPI,		0X40008000);

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
