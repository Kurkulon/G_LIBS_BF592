#include "spi.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 			S_SPIM::_busy_mask = 0;
u32 			S_SPIM::_alloc_mask = 0;

const byte		S_SPIM::_spi_pid[SPI_NUM]	= {	PID_DMA5_SPI0_RX_TX,	PID_DMA6_SPI1_RX_TX };
SPIHWT const	S_SPIM::_spi_hw[SPI_NUM]	= { HW::SPI0,				HW::SPI1			};

//S_SPIM *S_SPIM::_spi0;
//S_SPIM *S_SPIM::_spi1;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#pragma interrupt
//void S_SPIM::SPI0_Handler()
//{ 
//	if (_spi0 != 0) 
//	{
//		_spi0->IRQ_Handler();
//	};
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#pragma interrupt
//void S_SPIM::SPI1_Handler()
//{ 
//	if (_spi1 != 0)
//	{
//		_spi1->IRQ_Handler(); 
//	};
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::InitHW()
{
	_PIO_CS->DirSet(_MASK_CS_ALL); 
	_PIO_CS->ClrFER(_MASK_CS_ALL); 

	if (_num == 0)
	{
		HW::PORTF->FER |= (PF13|PF14|PF15) & _MASK_SCK_MOSI_MISO;
		HW::PORTF->MUX &= ~((PF13|PF14|PF15) &_MASK_SCK_MOSI_MISO);

		//_spi0 = this;

		//InitIVG(_ivg, _pid, SPI0_Handler); 
	}
	else
	{
		HW::PORTG->FER |= (PG8|PG9|PG10) & _MASK_SCK_MOSI_MISO;
		HW::PORTG->MUX &= ~((PG8|PG9|PG10) & _MASK_SCK_MOSI_MISO);

		//_spi1 = this;

		//InitIVG(_ivg, _pid, SPI1_Handler); 
	};

	//SIC_DisableIRQ(_pid);

	_PIO_CS->SET(_MASK_CS_ALL); 

	_hw->Baud = _baud;
	_hw->Ctl = _ctl;
	_hw->Stat = ~0;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::Disconnect()
{
	#ifdef CPU_SAME53	
		
		T_HW::S_SPI* spi = _uhw.spi;
		
		_PIO_SPCK->SetWRCONFIG(_MASK_SPCK, PORT_WRPMUX);
		_PIO_MOSI->SetWRCONFIG(_MASK_MOSI, PORT_WRPMUX);
		_PIO_MISO->SetWRCONFIG(_MASK_MISO, PORT_WRPMUX);
		_PIO_CS->SET(_MASK_CS_ALL); 
		_PIO_CS->DIRCLR = _MASK_CS_ALL; 
		_PIO_CS->SetWRCONFIG(_MASK_CS_ALL, PORT_PULLEN|PORT_WRPINCFG|PORT_WRPMUX);

		spi->CTRLA = SPI_SWRST;

		while(spi->SYNCBUSY);

		HW::MCLK->ClockDisable(_upid);

		Usic_Disconnect();

	#elif defined(CPU_XMC48)


	
	#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool S_SPIM::Connect(u32 baudrate)
{
	using namespace HW;

	if ((_alloc_mask & (1UL<<_num)) || _MASK_CS == 0 || _MASK_CS_LEN == 0)
	{
		return false;
	};

	_alloc_mask |= (1UL<<_num);
	
	_hw = _spi_hw[_num];

	if (baudrate == 0) baudrate = 1;

	_MASK_CS_ALL = 0;

	for (u32 i = 0; i < _MASK_CS_LEN; i++) _MASK_CS_ALL |= _MASK_CS[i];

	u16 baud = (_GEN_CLK + baudrate) / (baudrate*2);

	if (baud < 2) baud = 2;

	_baud = baud;

	InitHW();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::WritePIO(void *data, u16 count)
{
	byte *p = (byte*)data;

	_hw->Ctl = SPE|MSTR|TDBR_CORE|(_spimode&(CPOL|CPHA|LSBF));	

	while (count != 0)
	{
		_hw->TDBR = *(p++); count--;

		while (_hw->Stat & TXS) HW::ResetWDT();
	};

	while((_hw->Stat & SPIF) == 0);

	_hw->Ctl = 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::WriteAsyncDMA(void *data, u16 count)
{
	_hw->Ctl = GM|MSTR|TDBR_DMA|(_spimode&(CPOL|CPHA|LSBF));

	_DMA.Write8(data, count);

	_hw->Ctl |= SPE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::WriteSyncDMA(void *data, u16 count)
{
	WriteAsyncDMA(data, count);

	while (!CheckWriteComplete());

	_hw->Ctl = 0;
	_DMA.Disable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::WriteAsyncDMA(void *data1, u16 count1, void *data2, u16 count2)
{
	_hw->Ctl = GM|MSTR|TDBR_DMA|(_spimode&(CPOL|CPHA|LSBF));

	_DMA.Write8(data1, count1, data2, count2);

	_hw->Ctl |= SPE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::WriteSyncDMA(void *data1, u16 count1, void *data2, u16 count2)
{
	WriteAsyncDMA(data1, count1, data2, count2);

	while (!CheckWriteComplete());

	_hw->Ctl = 0;
	_DMA.Disable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::ReadPIO(void *data, u16 count)
{
	volatile register byte t;
	byte *p = (byte*)data;

	_DMA.Disable();

	_hw->Ctl = SPE|MSTR|SZ|RDBR_CORE|(_spimode&(CPOL|CPHA|LSBF));	

	t = _hw->RDBR; 

	while (count != 0)
	{
		while ((_hw->Stat & RXS) == 0);

		*(p++) = _hw->RDBR; count--;
	};

	_hw->Ctl = 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::ReadAsyncDMA(void *data, u16 count)
{
	_hw->Ctl = MSTR|SZ|RDBR_DMA|(_spimode&(CPOL|CPHA|LSBF));

	_DMA.Read8(data, count);

	_hw->Ctl |= SPE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::ReadSyncDMA(void *data, u16 count)
{
	ReadAsyncDMA(data, count);

	while (!CheckReadComplete());

	_hw->Ctl = 0;
	_DMA.Disable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

byte S_SPIM::WriteReadByte(byte v)
{
	_hw->Ctl = SPE|MSTR|TDBR_CORE|(_spimode&(CPOL|CPHA|LSBF));

	_hw->TDBR = v;

	while((*pSPI0_STAT & (SPIF|RXS)) != (SPIF|RXS)) HW::ResetWDT();

	return _hw->RDBR; 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool S_SPIM::AddRequest(DSCSPI *d)
{
	if (d == 0) { return false; };

	if (d->csnum >= _MASK_CS_LEN) return false;

	d->next = 0;
	d->ready = false;

	if (d->baud < 2) d->baud = 2;

	_reqList.Add(d);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool S_SPIM::Update()
{
	bool result = false;

	//HW::PIOG->SET(PG3);

	switch (_state)
	{
		case ST_WAIT:	//++++++++++++++++++++++++++++++++++++++++++++++
		{
			_dsc = _reqList.Get();

			if (_dsc != 0)
			{
				_hw->Ctl = 0;

				ChipSelect(_dsc->csnum, _dsc->mode, _dsc->baud);  //_PIO_CS->CLR(_MASK_CS[_dsc->csnum]);

				if (_dsc->alen == 0)
				{
					if (_dsc->wdata != 0 && _dsc->wlen > 0)
					{
						WriteAsyncDMA(_dsc->wdata, _dsc->wlen);

						_state = ST_WRITE; 
					}
					else if (_dsc->rdata != 0 && _dsc->rlen > 0)
					{
						ReadAsyncDMA(_dsc->rdata, _dsc->rlen);

						_state = ST_READ; 
					};
				}
				else
				{
					WriteAsyncDMA(&_dsc->adr, _dsc->alen, _dsc->wdata, _dsc->wlen);

					_state = ST_WRITE; 
				};
			};

			break;
		};

		case ST_WRITE:	//++++++++++++++++++++++++++++++++++++++++++++++
		{
			if (CheckWriteComplete())
			{
				_DMA.Disable();

				if (_dsc->rdata != 0 && _dsc->rlen > 0)
				{
					ReadAsyncDMA(_dsc->rdata, _dsc->rlen);

					_state = ST_READ; 
				}
				else
				{
					_state = ST_STOP; 
				};
			};

			break;
		};

		case ST_READ:	//++++++++++++++++++++++++++++++++++++++++++++++
		{
			if (CheckReadComplete())
			{
				_hw->Ctl = 0;
				_DMA.Disable();

				_state = ST_STOP; 
			};

			break;
		};

		case ST_STOP:	//++++++++++++++++++++++++++++++++++++++++++++++
		{
			_dsc->ready = true;

			ChipDisable();

			_dsc = 0;

			_state = ST_WAIT; 

			break;
		};
	};

	//HW::PIOG->CLR(PG3);

	return result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
