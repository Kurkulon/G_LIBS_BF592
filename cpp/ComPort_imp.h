//#include <stdio.h>
//#include <conio.h>

//#include "hardware.h"
#include "ComPort.h"
#include "bf592.h"
//#include "hardware.h"

#ifdef _DEBUG_
//	static const bool _debug = true;
#else
//	static const bool _debug = false;
#endif

extern dword millisecondsCount;

//#define MASKRTS (1<<5)

#pragma optimize_for_speed
//#pragma optimize_as_cmd_line

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Connect(dword speed, byte parity)
{
	if (_connected)
	{
		return false;
	};

	_BaudRateRegister = BoudToPresc(speed);

	_ModeRegister = WLS(8);

	switch (parity)
	{
		case 0:		// ��� ��������
			_ModeRegister |= 0;
			break;

		case 1:
			_ModeRegister |= PEN;
			break;

		case 2:
			_ModeRegister |= PEN|EPS;
			break;
	};

	*pUART0_GCTL = UCEN;
	*pUART0_LCR = _ModeRegister;
	SetBoudRate(_BaudRateRegister);

	*pPORTF_MUX	&= ~(PF11|PF12);	
	*pPORTF_FER |= PF11|PF12;	

	PIO_RTS_FER &= ~MASK_RTS;
	PIO_RTS_DIR |= MASK_RTS;
	PIO_RTS_CLR  = MASK_RTS;

	_status485 = READ_END;

	return _connected = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Disconnect()
{
	if (!_connected) return false;

	DisableReceive();
	DisableTransmit();

	*pUART0_GCTL = 0;

	_status485 = READ_END;

	_connected = false;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

word ComPort::BoudToPresc(dword speed)
{
	if (speed == 0) return 0;

	word presc;

	presc = (word)((SCLK + (speed<<3)) / (speed << 4));

	return presc;
}

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

void ComPort::SetBoudRate(word presc)
{
	union { word w; byte b[2]; };

	w = presc;

	*pUART0_LCR |= DLAB;
	*pUART0_DLL = b[0];
	*pUART0_DLH = b[1];
	*pUART0_LCR &= ~DLAB;
}

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::EnableTransmit(void* src, word count)
{
	PIO_RTS_SET = MASK_RTS;

	*pDMA8_CONFIG = 0;	// Disable transmit and receive
	*pUART0_IER = 0;

	*pDMA8_START_ADDR = src;
	*pDMA8_X_COUNT = count;
	*pDMA8_X_MODIFY = 1;
	*pDMA8_CONFIG = FLOW_STOP|WDSIZE_8|SYNC|DMAEN;

	_startTransmitTime.Reset();

	*pUART0_IER = ETBEI;

	_status485 = WRITEING;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::DisableTransmit()
{
	*pDMA8_CONFIG = 0;	// Disable transmit and receive
	*pUART0_IER = 0;

	PIO_RTS_CLR = MASK_RTS;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::EnableReceive(void* dst, word count)
{
	PIO_RTS_CLR = MASK_RTS;

	*pDMA7_CONFIG = 0;	// Disable transmit and receive
	*pUART0_IER = 0;

	*pDMA7_START_ADDR = dst;
	*pDMA7_CURR_X_COUNT = *pDMA7_X_COUNT = count;
	*pDMA7_X_MODIFY = 1;
	*pDMA7_CONFIG = WNR|FLOW_STOP|WDSIZE_8|SYNC|DMAEN;

	_startReceiveTime.Reset();

	count = *pUART0_RBR;
	count = *pUART0_LSR;
	*pUART0_IER = ERBFI;

	_status485 = WAIT_READ;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::DisableReceive()
{
	*pDMA7_CONFIG = 0;	// Disable transmit and receive
	*pUART0_IER = 0;

	PIO_RTS_CLR = MASK_RTS;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Update()
{
	static u32 stamp = 0;

	bool r = true;

	if (!_connected) 
	{
		_status485 = READ_END;
	};

	//stamp = GetCycles32();

	switch (_status485)
	{
		case WRITEING:

			if (*pUART0_LSR & TEMT)
			{
				_pWriteBuffer->transmited = true;
				_status485 = READ_END;

				DisableTransmit();
				DisableReceive();

				r = false;
			};

			break;

		case WAIT_READ:

			if ((_prevDmaCounter-*pDMA7_CURR_X_COUNT) == 0)
			{
				if (_startReceiveTime.Timeout(_preReadTimeout))
				{
					DisableReceive();
					_pReadBuffer->len = _pReadBuffer->maxLen - _prevDmaCounter;
					_pReadBuffer->recieved = _pReadBuffer->len > 0;
					_status485 = READ_END;
					r = false;
				};
			}
			else
			{
				_prevDmaCounter = *pDMA7_CURR_X_COUNT;
				_startReceiveTime.Reset();
				_preReadTimeout = _postReadTimeout;
			};

			break;

		case READ_END:

			r = false;

			break;
	};

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Read(ComPort::ReadBuffer *readBuffer, dword preTimeout, dword postTimeout)
{
	if (_status485 != READ_END || readBuffer == 0)
	{
		return false;
	};

	_preReadTimeout = preTimeout;
	_postReadTimeout = postTimeout;

	_pReadBuffer = readBuffer;
	_pReadBuffer->recieved = false;
	_pReadBuffer->len = 0;

	_prevDmaCounter = _pReadBuffer->maxLen;

	EnableReceive(_pReadBuffer->data, _pReadBuffer->maxLen);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Write(ComPort::WriteBuffer *writeBuffer)
{
	if (_status485 != READ_END || writeBuffer == 0)
	{
		return false;
	};

	_pWriteBuffer = writeBuffer;
	_pWriteBuffer->transmited = false;

	EnableTransmit(_pWriteBuffer->data, _pWriteBuffer->len);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

