#ifndef TWI_IMP_H__24_11_2022__18_29
#define TWI_IMP_H__24_11_2022__18_29

#include "types.h"
#include "bf592.h"
#include "twi.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u16 twiWriteCount = 0;
static u16 twiReadCount = 0;
static byte *twiWriteData = 0;
static byte *twiReadData = 0;
static DSCTWI* twi_dsc = 0;
static DSCTWI* twi_lastDsc = 0;

EX_REENTRANT_HANDLER(TWI_ISR)
{
	u16 stat = *pTWI_INT_STAT;

	if (stat & RCVSERV)
	{
		if (twiReadCount > 0)
		{
			*twiReadData++ = *pTWI_RCV_DATA8;
			twiReadCount--;
		};

		if (twiReadCount == 0)
		{
			//*pTWI_INT_MASK = MERR|MCOMP;
			*pTWI_MASTER_CTL |= STOP;
			*pTWI_FIFO_CTL = XMTFLUSH|RCVFLUSH;
		};
	};
	
	if (stat & XMTSERV)
	{
		if (twiWriteCount == 0 && twi_dsc->wlen2 != 0)
		{
			twiWriteData = (byte*)twi_dsc->wdata2;
			twiWriteCount = twi_dsc->wlen2;
			twi_dsc->wlen2 = 0;
		};

		if (twiWriteCount > 0)
		{
			*pTWI_XMT_DATA8 = *twiWriteData++;
			twiWriteCount--;

		};
		//else if (twiReadCount > 0)
		//{
		//	*pTWI_INT_MASK = MERR|MCOMP;
		//	*pTWI_MASTER_CTL |= RSTART|MDIR;
		//}
	};
	
	//if (stat & MERR)
	//{
	//	*pTWI_INT_STAT = MERR;
	//};

	if (stat & (MCOMP|MERR))
	{
		twi_dsc->ack = ((stat & MERR) == 0);

		if (twi_dsc->ack && twiReadCount > 0)
		{
			*pTWI_INT_MASK = RCVSERV|MERR|MCOMP;
			*pTWI_MASTER_CTL = ((twiReadCount<<6)&DCNT)|MDIR|FAST|MEN;
		}
		else
		{
			twi_dsc->ready = true;
			twi_dsc->readedLen = twi_dsc->rlen - twiReadCount;
			twi_dsc->master_stat = *pTWI_MASTER_STAT;

			DSCTWI *ndsc = twi_dsc->next;

			if (ndsc != 0)
			{
				twi_dsc->next = 0;
				twi_dsc = ndsc;

				twi_dsc->ready = false;
				twi_dsc->ack = false;
				twi_dsc->readedLen = 0;

				if (twi_dsc->wdata2 == 0) twi_dsc->wlen2 = 0;

				twiWriteData = (byte*)twi_dsc->wdata;
				twiWriteCount = twi_dsc->wlen;
				twiReadData = (byte*)twi_dsc->rdata;
				twiReadCount = twi_dsc->rlen;

				u16 len = twiWriteCount + twi_dsc->wlen2;

				*pTWI_MASTER_STAT = ~0;
				*pTWI_FIFO_CTL = 0;

				*pTWI_MASTER_ADDR = twi_dsc->adr;

				if (len != 0)
				{
					*pTWI_XMT_DATA8 = *twiWriteData++; twiWriteCount--;
					*pTWI_INT_MASK = XMTSERV|MERR|MCOMP;
					*pTWI_MASTER_CTL = ((len<<6)&DCNT)|FAST|MEN|((twiReadCount>0) ? RSTART : 0);
				}
				else
				{
					*pTWI_INT_MASK = RCVSERV|MERR|MCOMP;
					*pTWI_MASTER_CTL = (twiReadCount<<6)|MDIR|FAST|MEN;
				};
			}
			else
			{
				*pTWI_MASTER_CTL = 0;
				*pTWI_MASTER_STAT = ~0;
				*pTWI_FIFO_CTL = XMTFLUSH|RCVFLUSH;

				*pTWI_INT_MASK = 0;

				twi_lastDsc = twi_dsc = 0;
			};

		};
	};

	*pTWI_INT_STAT = stat;

	ssync();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitTWI()
{
	*pTWI_CONTROL = TWI_ENA | (SCLK_MHz/10);
	*pTWI_CLKDIV = CLKHI(150/(SCLK_MHz/10))|CLKLOW(150/(SCLK_MHz/10));
	*pTWI_INT_MASK = 0;
	*pTWI_MASTER_ADDR = 0;

	InitIVG(IVG_TWI, PID_TWI, TWI_ISR);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TWI_Write(DSCTWI *d)
{
//	using namespace HW;

	if (twi_dsc != 0 || d == 0) { return false; };
	if ((d->wdata == 0 || d->wlen == 0) && (d->rdata == 0 || d->rlen == 0)) { return false; }

	twi_dsc = d;

	twi_dsc->ready = false;
	twi_dsc->ack = false;
	twi_dsc->readedLen = 0;

	if (twi_dsc->wdata2 == 0) twi_dsc->wlen2 = 0;

	u32 t = cli();

	*pTWI_MASTER_CTL = 0;
	*pTWI_MASTER_STAT = ~0;
	*pTWI_FIFO_CTL = 0;//XMTINTLEN|RCVINTLEN;

	twiWriteData = (byte*)twi_dsc->wdata;
	twiWriteCount = twi_dsc->wlen;
	twiReadData = (byte*)twi_dsc->rdata;
	twiReadCount = twi_dsc->rlen;

	u16 len = twiWriteCount + twi_dsc->wlen2;

	*pTWI_MASTER_ADDR = twi_dsc->adr;

	if (len != 0)
	{
		*pTWI_XMT_DATA8 = *twiWriteData++; twiWriteCount--;
		*pTWI_INT_MASK = XMTSERV|MERR|MCOMP;
		*pTWI_MASTER_CTL = ((len<<6)&DCNT)|FAST|MEN|((twiReadCount>0) ? RSTART : 0);
	}
	else
	{
		*pTWI_INT_MASK = RCVSERV|MERR|MCOMP;
		*pTWI_MASTER_CTL = (twiReadCount<<6)|MDIR|FAST|MEN;
	};

	sti(t);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TWI_AddRequest(DSCTWI *d)
{
	if (d == 0) { return false; };
	if ((d->wdata == 0 || d->wlen == 0) && (d->rdata == 0 || d->rlen == 0)) { return false; }

	d->next = 0;
	d->ready = false;

	if (d->wdata2 == 0) d->wlen2 = 0;

	u32 t = cli();

	if (twi_lastDsc == 0)
	{
		twi_lastDsc = d;

		sti(t);

		return TWI_Write(d);
	}
	else
	{
		twi_lastDsc->next = d;
		twi_lastDsc = d;

		sti(t);
	};

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // TWI_IMP_H__24_11_2022__18_29
