#include "spi.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 			S_SPIM::_busy_mask = 0;
u32 			S_SPIM::_alloc_mask = 0;

const byte		S_SPIM::_spi_pid[SPI_NUM]	= {	PID_DMA5_SPI0_RX_TX,	PID_DMA6_SPI1_RX_TX };
SPIHWT const	S_SPIM::_spi_hw[SPI_NUM]	= { HW::SPI0,				HW::SPI1			};

S_SPIM *S_SPIM::_spi0;
S_SPIM *S_SPIM::_spi1;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma interrupt
void S_SPIM::SPI0_Handler()
{ 
	if (_spi0 != 0) 
	{
		_spi0->IRQ_Handler();
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma interrupt
void S_SPIM::SPI1_Handler()
{ 
	if (_spi1 != 0)
	{
		_spi1->IRQ_Handler(); 
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::InitHW()
{
	_PIO_CS->Dir |= _MASK_CS_ALL; 
	_PORT_CS->FER &= ~_MASK_CS_ALL; 

	if (_num == 0)
	{
		HW::PORTF->FER |= PF13|PF14|PF15;
		HW::PORTF->MUX &= ~(PF13|PF14|PF15);

		_spi0 = this;

		InitIVG(_ivg, _pid, SPI0_Handler); 
	}
	else
	{
		HW::PORTF->FER |= PG8|PG9|PG10;
		HW::PORTF->MUX &= ~(PG8|PG9|PG10);

		_spi1 = this;

		InitIVG(_ivg, _pid, SPI1_Handler); 
	};

	SIC_DisableIRQ(_pid);

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
	_hw->Ctl = MSTR|TDBR_DMA|(_spimode&(CPOL|CPHA|LSBF));

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
	_hw->Ctl = MSTR|TDBR_DMA|(_spimode&(CPOL|CPHA|LSBF));

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

	u32 t = cli();

	if (_dsc == 0)
	{
		Write(d);
	}
	else
	{
		_reqList.Add(d);
	};

	sti(t);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::Write(DSCSPI *dsc)
{
	_hw->Ctl = 0;
	_irqCount = 0;
	_dsc = dsc;

	ChipSelect(dsc->csnum, dsc->mode);  //_PIO_CS->CLR(_MASK_CS[_dsc->csnum]);

	_hw->Baud = dsc->baud;

	if (dsc->alen == 0)
	{
		if (dsc->wdata != 0 && dsc->wlen > 0)
		{
			WriteAsyncDMA(dsc->wdata, dsc->wlen);

			_state = ST_WRITE; 
		}
		else if (dsc->rdata != 0 && dsc->rlen > 0)
		{
			ReadAsyncDMA(dsc->rdata, dsc->rlen);

			_state = ST_STOP; 
		};
	}
	else
	{
		WriteAsyncDMA(&dsc->adr, dsc->alen, dsc->wdata, dsc->wlen);

		_state = ST_WRITE; 
	};

	SIC_EnableIRQ(_pid);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void S_SPIM::IRQ_Handler()
{
	_irqCount += 1;

	switch (_state)
	{
		case ST_WRITE:
		{
			DSCSPI &dsc = *_dsc;

			if (CheckWriteComplete())
			{
				_DMA.Disable();

				if (dsc.rdata != 0 && dsc.rlen > 0)
				{
					ReadAsyncDMA(dsc.rdata, dsc.rlen);
				};

				_state = ST_STOP; 
			};

			break;
		};

		case ST_STOP:
		{
			if (CheckReadComplete())
			{
				_dsc->ready = true;
				
				_dsc = _reqList.Get();
				
				ChipDisable();//_PIO_CS->SET(_MASK_CS_ALL);

				_DMA.Disable();

				_hw->Ctl = 0;

				if (_dsc != 0)
				{
					Write(_dsc);
				}
				else
				{
					_state = ST_WAIT; 

					SIC_DisableIRQ(_pid);
				};
			};

			break;
		};
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool S_SPIM::Update()
{
	bool result = false;

#ifdef CPU_SAME53

	T_HW::S_SPI* spi = _uhw.spi;

	switch (_state)
	{
		case WAIT:

			if (CheckReset())
			{
				Usic_Update();
			}
			else
			{
				_dsc = _reqList.Get();

				if (_dsc != 0)
				{
					Usic_Lock();

					ChipSelect(_dsc->csnum);  //_PIO_CS->CLR(_MASK_CS[_dsc->csnum]);

					DSCSPI &dsc = *_dsc;

					dsc.ready = false;

					if (dsc.alen == 0)
					{
						if (dsc.wdata != 0 && dsc.wlen > 0)
						{
							WriteAsyncDMA(dsc.wdata, dsc.wlen);
							//_DMATX->WritePeripheral(dsc.wdata, &spi->DATA, dsc.wlen, DMCH_TRIGACT_BURST|(((DMCH_TRIGSRC_SERCOM0_TX>>8)+_usic_num*2)<<8), DMDSC_BEATSIZE_BYTE);

							_state = WRITE; 
						}
						else if (dsc.rdata != 0 && dsc.rlen > 0)
						{
							ReadAsyncDMA(dsc.rdata, dsc.rlen);
							//spi->CTRLB |= SPI_RXEN;
							//_DMARX->ReadPeripheral(&spi->DATA, dsc.rdata, dsc.rlen, DMCH_TRIGACT_BURST|(((DMCH_TRIGSRC_SERCOM0_RX>>8)+_usic_num*2)<<8), DMDSC_BEATSIZE_BYTE);
							//_DMATX->WritePeripheral(dsc.rdata, &spi->DATA, dsc.rlen+1, DMCH_TRIGACT_BURST|(((DMCH_TRIGSRC_SERCOM0_TX>>8)+_usic_num*2)<<8), DMDSC_BEATSIZE_BYTE);

							_state = STOP; 
						};
					}
					else
					{
						spi->INTFLAG = ~0;
						spi->INTENCLR = ~0;
						spi->CTRLB &= ~SPI_RXEN;

						_DMATX->WritePeripheral(&dsc.adr, &spi->DATA, dsc.alen, dsc.wdata, dsc.wlen, DMCH_TRIGACT_BURST|(((DMCH_TRIGSRC_SERCOM0_TX>>8)+_usic_num*2)<<8), DMDSC_BEATSIZE_BYTE);

						_state = WRITE; 
					};
				};
			};

			break;

		case WRITE:
		{
			DSCSPI &dsc = *_dsc;

			if (CheckWriteComplete())
			{
				_DMATX->Disable();

				if (dsc.rdata != 0 && dsc.rlen > 0)
				{
					ReadAsyncDMA(dsc.rdata, dsc.rlen);

					//spi->CTRLB |= SPI_RXEN;

					//_DMARX->ReadPeripheral(&spi->DATA, dsc.rdata, dsc.rlen, DMCH_TRIGACT_BURST|(((DMCH_TRIGSRC_SERCOM0_RX>>8)+_usic_num*2)<<8), DMDSC_BEATSIZE_BYTE);
					//_DMATX->WritePeripheral(dsc.rdata, &spi->DATA, dsc.rlen+1, DMCH_TRIGACT_BURST|(((DMCH_TRIGSRC_SERCOM0_TX>>8)+_usic_num*2)<<8), DMDSC_BEATSIZE_BYTE);
				};

				_state = STOP; 
			};

			break;
		};

		case STOP:
		{
			if (CheckReadComplete())
			{
				_dsc->ready = true;
				
				_dsc = 0;
				
				ChipDisable();//_PIO_CS->SET(_MASK_CS_ALL);

				_DMARX->Disable();
				_DMATX->Disable();

				spi->CTRLB &= ~SPI_RXEN;
				spi->INTFLAG = ~0;
				spi->INTENCLR = ~0;

				_state = WAIT; 

				Usic_Unlock();
			};

			break;
		};
	};

#elif defined(CPU_XMC48)

	USICHWT	&spi = _uhw;

	switch (_state)
	{
		case WAIT:

			if (CheckReset())
			{
				Usic_Update();
			}
			else
			{
				_dsc = _reqList.Get();

				if (_dsc != 0)
				{
					Usic_Lock();

					ChipSelect(_dsc->csnum);  //_PIO_CS->CLR(_MASK_CS[_dsc->csnum]);

					_DMA->SetDlrLineNum(_DRL);

					DSCSPI &dsc = *_dsc;

					dsc.ready = false;

					if (dsc.alen == 0)
					{
						if (dsc.wdata != 0 && dsc.wlen > 0)
						{
							WriteAsyncDMA(dsc.wdata, dsc.wlen);

							_state = WRITE; 
						}
						else 
						{
							if (dsc.rdata != 0 && dsc.rlen > 0) ReadAsyncDMA(dsc.rdata, dsc.rlen);

							_state = STOP; 
						};
					}
					else
					{
						WriteAsyncDMA(&dsc.adr, dsc.alen);

						_state = WRITE_ADR; 
					};
				};
			};

			break;

		case WRITE_ADR:
		{
			DSCSPI &dsc = *_dsc;

			u32 psr = spi->PSR_SSCMode;

			if (/*CheckWriteComplete() && */(psr & SPI_MSLS) == 0)
			{
				_DMA->Disable();

				if (dsc.wdata != 0 && dsc.wlen > 0)
				{
					WriteAsyncDMA(dsc.wdata, dsc.wlen);

					_state = WRITE; 
				}
				else 
				{
					if (dsc.rdata != 0 && dsc.rlen > 0) ReadAsyncDMA(dsc.rdata, dsc.rlen);

					_state = STOP; 
				};
			};

			break;
		};

		case WRITE:
		{
			DSCSPI &dsc = *_dsc;

			//u32 psr = spi->PSR_SSCMode;

			if (/*CheckWriteComplete() && */(spi->PSR_SSCMode & SPI_MSLS) == 0)
			{
				_DMA->Disable();

				if (dsc.rdata != 0 && dsc.rlen > 0)	ReadAsyncDMA(dsc.rdata, dsc.rlen);

				_state = STOP; 
			};

			break;
		};

		case STOP:
		{
			if (CheckReadComplete())
			{
				_dsc->ready = true;
				
				_dsc = 0;
				
				ChipDisable();//_PIO_CS->SET(_MASK_CS_ALL);

				_DMA->Disable();

				spi->TCSR = SPI__TCSR|USIC_TDSSM(1);
				spi->CCR = SPI__CCR;
				spi->PCR_SSCMode = SPI__PCR;

				_state = WAIT; 

				Usic_Unlock();
			};

			break;
		};
	};

#endif

	return result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
