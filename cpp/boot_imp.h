#ifndef BOOT_IMP_H__14_11_2022__12_13
#define BOOT_IMP_H__14_11_2022__12_13

#include "types.h"
#include "bf592.h"

#include "ComPort.h"
#include "CRC16.h"
#include "at25df021.h"

static ComPort com;

static u16 manReqWord = BOOT_MAN_REQ_WORD;
static u16 manReqMask = BOOT_MAN_REQ_MASK;

//static u16 numDevice = 1;
//static u16 verDevice = 0x101;

static u32 manCounter = 0;
static u32 err06 = 0;


static u32 timeOut = MS2RT(500);
//static bool runMainApp = false;

static u16 flashCRC = 0;
static u32 flashLen = 0;
static u16 lastErasedBlock = ~0;
static bool flashOK = false;
static bool flashChecked = false;
static bool flashCRCOK = false;

static RTM32 tm32;

static void CheckFlash();

static u32 curWriteReqAdr = 0;

static byte buf[SECTOR_SIZE];

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef BOOT_NETADR

extern u16 GetNetAdr();

struct  ReqDsp05 { u16 adr; u16 rw; u16 crc; };												// запрос контрольной суммы и длины программы во флэш-памяти
struct  ReqDsp06 { u16 adr; u16 rw; u16 stAdr; u16 len; byte data[256]; u16 crc; };			// запись страницы во флэш
struct  ReqDsp07 { u16 adr; u16 rw; word crc; };											// перезагрузить блэкфин

struct  RspDsp05 { u16 adr; u16 rw; u16 flashLen; u32 startAdr; u16 flashCRC; u16 crc; };	// запрос контрольной суммы и длины программы во флэш-памяти
struct  RspDsp06 { u16 adr; u16 rw; u16 res; u16 crc; };									// запись страницы во флэш

#else

struct  ReqDsp05 { u16 rw; u16 crc; };												// запрос контрольной суммы и длины программы во флэш-памяти
struct  ReqDsp06 { u16 rw; u16 stAdr; u16 len; byte data[256]; u16 crc; };			// запись страницы во флэш
struct  ReqDsp07 { u16 rw; word crc; };												// перезагрузить блэкфин

struct  RspDsp05 { u16 rw; u16 flashLen; u32 startAdr; u16 flashCRC; u16 crc; };	// запрос контрольной суммы и длины программы во флэш-памяти
struct  RspDsp06 { u16 rw; u16 res; u16 crc; };										// запись страницы во флэш

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RunMainApp()
{
	if (!flashChecked) CheckFlash();

	if (flashOK && flashCRCOK) HW::DisableWDT(), bfrom_SpiBoot(FLASH_START_ADR, BFLAG_PERIPHERAL | BFLAG_NOAUTO | BFLAG_FASTREAD | BFLAG_TYPE3 | 7, 0, 0);
	
	tm32.Reset(); timeOut = MS2RT(1000);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFunc_05(Req *r, ComPort::WriteBuffer *wb)
{
	const ReqDsp05 *req = (ReqDsp05*)(r->GetDataPtr());
	static RspDsp05 rsp;

	#ifdef BOOT_NETADR

		if (req->adr == 0 || r->len < sizeof(ReqDsp05)) return  false;
	
		rsp.adr = req->adr;

	#else

		if (r->len < sizeof(ReqDsp05)) return  false;

	#endif

	if ((req->rw & manReqMask) != manReqWord || r->len < 2) return false;

	rsp.rw = req->rw;
	rsp.flashLen = flashLen;
	rsp.flashCRC = flashCRC;
	rsp.startAdr = FLASH_START_ADR;

	rsp.crc = GetCRC16(&rsp, sizeof(rsp)-2);

	wb->data = &rsp;
	wb->len = sizeof(rsp);

	FreeReq(r);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFunc_06(Req *r, ComPort::WriteBuffer *wb)
{
	const ReqDsp06 *req = (ReqDsp06*)(r->GetDataPtr());
	static RspDsp06 rsp;

	u16 xl = req->len + sizeof(ReqDsp06) - sizeof(req->data);

	if (r->len < xl) return  false;

	if (req->stAdr >= curWriteReqAdr)
	{
		curWriteReqAdr = req->stAdr + req->len;
	
		r->dataOffset = req->data - r->data;
		r->stAdr = req->stAdr;
		r->len = req->len;

		FlashWriteReq(r);
	};

	#ifdef BOOT_NETADR

		if (req->adr == 0)	return false;

		rsp.adr = req->adr;

	#endif

	rsp.res = GetLastError();

	rsp.rw = req->rw;

	rsp.crc = GetCRC16(&rsp, sizeof(rsp)-2);

	wb->data = &rsp;
	wb->len = sizeof(rsp);

	flashChecked = flashOK = flashCRCOK = false;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFunc(Req *r, ComPort::WriteBuffer *wb)
{
	bool result = false;

	ReqDsp05 *req = (ReqDsp05*)r->GetDataPtr();

	u16 t = req->rw;

	#ifdef BOOT_NETADR
	if ((req->adr != GetNetAdr() && req->adr != 0) || (t & manReqMask) != manReqWord || r->len < 2)
	#else
	if ((t & manReqMask) != manReqWord || r->len < 2)
	#endif
	{
		return false;
	};

	manCounter += 1;

	t &= 0xFF;

	switch (t)
	{
		case 5: 	result = RequestFunc_05(r, wb); break;
		case 6: 	result = RequestFunc_06(r, wb); break;

		case 7: 		
		default:	RunMainApp(); break;
	};

	if (result)	tm32.Reset(), timeOut = MS2RT(10000);

	return result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateBlackFin()
{
	static byte i = 0;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	
	static Req *req = 0;

	ResetWDT();

	switch(i)
	{
		case 0:

			req = AllocReq();

			if (req != 0)
			{
				req->len = 0;
				req->dataOffset = 0;
				rb.data = req->GetDataPtr();
				rb.maxLen = req->MaxLen();
				com.Read(&rb, BOOT_COM_PRETIMEOUT, BOOT_COM_POSTTIMEOUT);
				i++;
			};

			break;

		case 1:

			if (!com.Update())
			{
				if (rb.recieved && rb.len > 0 && GetCRC16(rb.data, rb.len) == 0)
				{
					tm32.Reset();

					req->len = rb.len;

					if (RequestFunc(req, &wb))
					{
						com.Write(&wb);
						i++;
					}
					else
					{
						FreeReq(req);

						req = 0;

						i = 0;
					};
				}
				else
				{
					FreeReq(req);

					req = 0;

					i = 0;
				};
			};

			break;

		case 2:

			if (!com.Update())
			{
				i = 0;
			};

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CheckFlash()
{
	static ADI_BOOT_HEADER bh;
	static u16 crc = 0;

	if (FlashBusy()) return;

	u32 *p = (u32*)&bh;

	u32 adr = 0;
	
	flashOK = flashChecked = flashCRCOK = false;

	at25df021_Read(buf, 0, sizeof(buf));

	while (1)
	{
		at25df021_Read(&bh, FLASH_START_ADR + adr, sizeof(bh));

		u32 xor = p[0] ^ p[1] ^ p[2] ^ p[3];
		xor ^= xor >> 16; 
		xor = (xor ^ (xor >> 8)) & 0xFF; 

		if (((u32)(bh.dBlockCode) >> 24) == 0xAD && xor == 0)
		{
			adr += sizeof(bh);

			if ((bh.dBlockCode & BFLAG_FILL) == 0)
			{
				adr += bh.dByteCount;	
			};

			if (bh.dBlockCode & BFLAG_FINAL)
			{
				flashOK = true;

				break;
			};
		}
		else
		{
			break;
		};
	};

	flashLen = adr;

	at25df021_Read(&crc, FLASH_START_ADR + adr, sizeof(crc));

	if (flashLen > 0) flashCRC = at25df021_GetCRC16(FLASH_START_ADR, flashLen), flashCRCOK = (flashCRC == crc);

	if (!flashCRCOK) flashLen = 0;

	flashChecked = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

i16 index_max = 0;

int main( void )
{
	static byte s = 0;
	static byte i = 0;

	static u32 pt = 0;

	//static RTM32 tm;

	InitHardware();

	FlashInit();

	com.Connect(BOOT_COM_SPEED, BOOT_COM_PARITY);

	CheckFlash();

	tm32.Reset(); timeOut = MS2RT(10000);

	while (1)
	{
		UpdateBlackFin();
		FlashUpdate();

		if (tm32.Check(timeOut)) RunMainApp();

		MAIN_LOOP_PIN_TGL();
	};

//	return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // BOOT_IMP_H__14_11_2022__12_13
