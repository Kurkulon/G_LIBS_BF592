#include "types.h"
#include "BF592.h"

#include <bfrom.h>
#include <ccblkfn.h>
#include <sysreg.h> 

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define CLKIN_MHz			25
#define CLKIN_DIV			2	// 1, 2

#define PLL_MUL				8	// 5...64
#define SCLK_DIV			1	// 1...15
#define CCLK_CSEL			0	// 0...3
#define CCLK_DIV			(1UL<<CCLK_CSEL)

#define VCO_CLK_MHz 		(CLKIN_MHz*PLL_MUL/CLKIN_DIV)
#define CCLK_MHz			VCO_CLK_MHz/CCLK_DIV
#define SCLK_MHz			VCO_CLK_MHz/SCLK_DIV

#define VRCTL_VALUE         0x0000

#if CLKIN_DIV == 2
#define PLLCTL_VALUE        (SET_MSEL(PLL_MUL)|DF)
#else
#define PLLCTL_VALUE        (SET_MSEL(PLL_MUL))
#endif

#define PLLDIV_VALUE        (SET_SSEL(SCLK_DIV))
#define PLLLOCKCNT_VALUE    0x0000
#define PLLSTAT_VALUE       0x0000
//#define RSICLK_DIV          0x0001


#define BAUD_RATE_DIVISOR 	5

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

section ("L1_code")

void initcode(ADI_BOOT_DATA* pBS)
{
    *pSIC_IWR0 = IRQ_PLL_WAKEUP;

	ADI_SYSCTRL_VALUES sysctrl = {	VRCTL_VALUE,
									PLLCTL_VALUE,		/* (25MHz CLKIN x (MSEL=16))::CCLK = 400MHz */
									PLLDIV_VALUE,		/* (400MHz/(SSEL=4))::SCLK = 100MHz */
									PLLLOCKCNT_VALUE,
									PLLSTAT_VALUE };


	bfrom_SysControl( SYSCTRL_WRITE | SYSCTRL_PLLCTL | SYSCTRL_PLLDIV, &sysctrl, 0);

	pBS->dFlags |= BFLAG_FASTREAD;
	*pSPI0_BAUD = BAUD_RATE_DIVISOR;
	pBS->dClock = BAUD_RATE_DIVISOR; /* required to keep dClock in pBS (-> ADI_BOOT_DATA) consistent */

    *pSIC_IWR0 = IWR0_ENABLE_ALL;

    sysreg_write(reg_LC0,0);
    sysreg_write(reg_LC1,0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

