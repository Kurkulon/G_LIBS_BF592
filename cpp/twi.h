#ifndef TWI_H__24_11_2022__18_40
#define TWI_H__24_11_2022__18_40

#include "types.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct DSCTWI
{
	DSCTWI*			next;
	void*			wdata;
	void*			rdata;
	void*			wdata2;
	u16				wlen;
	u16				wlen2;
	u16				rlen;
	u16				readedLen;
	u16				master_stat;
	byte			adr;
	volatile bool	ready;
	volatile bool	ack;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern bool TWI_AddRequest(DSCTWI *d);

#endif // TWI_H__24_11_2022__18_40
