#ifndef SYSTEM_IMP_H__14_11_2022__11_53
#define SYSTEM_IMP_H__14_11_2022__11_53

#include "types.h"
#include "bf592.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitIVG(u32 IVG, u32 PID, void (*EVT)())
{
	if (IVG <= 15)
	{
		*(pEVT0 + IVG) = (void*)EVT;
		*pIMASK |= 1<<IVG; 

		if (IVG > 6)
		{
			IVG -= 7;

			byte n = PID/8;
			byte i = (PID&7)*4;

			pSIC_IAR0[n] = (pSIC_IAR0[n] & ~(0xF<<i)) | (IVG<<i);

			*pSIC_IMASK |= 1<<PID;
		};
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_PLL()
{
	u32 SIC_IWR1_reg;                /* backup SIC_IWR1 register */

	/* use Blackfin ROM SysControl() to change the PLL */
    ADI_SYSCTRL_VALUES sysctrl = {	VRCTL_VALUE,
									PLLCTL_VALUE,		/* (25MHz CLKIN x (MSEL=16))::CCLK = 400MHz */
									PLLDIV_VALUE,		/* (400MHz/(SSEL=4))::SCLK = 100MHz */
									PLLLOCKCNT_VALUE,
									PLLSTAT_VALUE };

	/* use the ROM function */
	bfrom_SysControl( SYSCTRL_WRITE | SYSCTRL_PLLCTL | SYSCTRL_PLLDIV, &sysctrl, 0);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LowLevelInit()
{
	Init_PLL();
																				
	*pPORTF_MUX			= INIT_PORTF_MUX		;	
	*pPORTG_MUX			= INIT_PORTG_MUX		;	
						  
	*pPORTF_FER 		= INIT_PORTF_FER 		;	
	*pPORTG_FER 		= INIT_PORTG_FER 		;	
						  
	*pPORTFIO_DIR 		= INIT_PORTFIO_DIR 		;		
	*pPORTGIO_DIR 		= INIT_PORTGIO_DIR 		;		
						  
	*pPORTFIO_INEN 		= INIT_PORTFIO_INEN 	;	
	*pPORTGIO_INEN 		= INIT_PORTGIO_INEN 	;	
						  
	*pPORTGIO 			= INIT_PORTGIO 			;
	*pPORTFIO 			= INIT_PORTFIO 			;
						  
	*pPORTFIO_POLAR		= INIT_PORTFIO_POLAR	;
	*pPORTFIO_EDGE		= INIT_PORTFIO_EDGE 	;
	*pPORTFIO_BOTH		= INIT_PORTFIO_BOTH 	;
	*pPORTFIO_MASKA 	= INIT_PORTFIO_MASKA	;
	*pPORTFIO_MASKB 	= INIT_PORTFIO_MASKB	;
						  
	*pPORTGIO_POLAR		= INIT_PORTGIO_POLAR	;
	*pPORTGIO_EDGE 		= INIT_PORTGIO_EDGE 	;
	*pPORTGIO_BOTH 		= INIT_PORTGIO_BOTH 	;
	*pPORTGIO_MASKA 	= INIT_PORTGIO_MASKA	;
	*pPORTGIO_MASKB 	= INIT_PORTGIO_MASKB	;

	*pWDOG_CNT 			= INIT_WDOG_CNT;
	*pWDOG_CTL 			= INIT_WDOG_CTL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // SYSTEM_IMP_H__11_10_2022__18_02
