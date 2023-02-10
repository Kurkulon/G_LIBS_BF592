#ifndef SPI_H__21_04_2022__11_18
#define SPI_H__21_04_2022__11_18

#include "types.h"
#include "bf592.h"
#include "list.h"
#include "DMA.h"


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define SPI_NUM 2

typedef T_HW::S_SPI *SPIHWT;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct DSCSPI
{
	DSCSPI*			next;

	void*			wdata;
	void*			rdata;
	u32				adr;
	u16				alen;
	u16				wlen;
	u16				rlen;
	u16				mode;
	u16				baud;
	volatile bool	ready;
	byte			csnum;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class S_SPIM
{
protected:

	//static  void			SPI0_Handler();
	//static  void			SPI1_Handler();

	//static S_SPIM *_spi0;
	//static S_SPIM *_spi1;

	static 	u32 			_busy_mask;
	static 	u32 			_alloc_mask;

	static  const byte		_spi_pid[SPI_NUM];

	static	SPIHWT	const	_spi_hw[SPI_NUM];

	//T_HW::S_PORT * const	_PORT_CS;
	T_HW::S_PIO * const		_PIO_CS;
	const u16 * const		_MASK_CS;
	const u32 				_GEN_CLK;
	const u16				_MASK_SCK_MOSI_MISO;

	const byte				_num;
	const byte				_pid;
	SPIHWT					_hw;

	const byte				_MASK_CS_LEN;

	DMA_CH					_DMA;

	List<DSCSPI>	_reqList;
	DSCSPI*			_dsc;

	enum STATE { ST_WAIT = 0, ST_WRITE, ST_READ, ST_STOP };

	STATE _state;

	u16		_baud;
	u16		_ctl;
	u16		_MASK_CS_ALL;
	u16		_spimode;

	//void IRQ_Handler();


public:

	S_SPIM(byte num, T_HW::S_PIO* piocs, u16* mcs, byte mcslen, u32 gen_clk, u16 mask_sck_mosi_miso = ~0)
		: _num(num), _pid(_spi_pid[num]), _PIO_CS(piocs), _MASK_CS(mcs), _MASK_CS_LEN(mcslen), _GEN_CLK(gen_clk),
		_MASK_SCK_MOSI_MISO(mask_sck_mosi_miso), _DMA(5+num), _dsc(0), _state(ST_WAIT), _spimode(0) {}

	//S_SPIM(byte num, T_HW::S_PORT* portcs, T_HW::S_PIO* piocs, u16* mcs, byte mcslen, u32 gen_clk, u16 mask_sck_mosi_miso = ~0)
	//	: _num(num), _pid(_spi_pid[num]), _PIO_CS(piocs), _MASK_CS(mcs), _MASK_CS_LEN(mcslen), _GEN_CLK(gen_clk),
	//	_MASK_SCK_MOSI_MISO(mask_sck_mosi_miso), _DMA(5+num), _dsc(0), _state(ST_WAIT), _spimode(0) {}

	bool CheckWriteComplete()	{ return _DMA.CheckComplete() && (_hw->Stat & (SPIF|TXS)) == SPIF; }
	bool CheckReadComplete()	{ if (_DMA.CheckComplete()) { _hw->Ctl = 0; _DMA.Disable(); return true;} else return false; }

	void ChipSelect(byte num, u16 spimode, u16 baud)	{ _hw->Baud = baud; _spimode = spimode & (CPOL|CPHA|LSBF); _PIO_CS->CLR(_MASK_CS[num]); }
	void ChipDisable()									{ _PIO_CS->SET(_MASK_CS_ALL); }


	bool Connect(u32 baudrate);
	void Disconnect();
	bool AddRequest(DSCSPI *d);
	bool Update();
	
	inline void SetMode(u16 mode) { _spimode = mode & (CPOL|CPHA|LSBF); }

	byte WriteReadByte(byte v);

	void WritePIO(void *data, u16 count);
	void WriteAsyncDMA(void *data, u16 count);
	void WriteSyncDMA(void *data, u16 count);

	void WriteAsyncDMA(void *data1, u16 count1, void *data2, u16 count2);
	void WriteSyncDMA(void *data1, u16 count1, void *data2, u16 count2);

	void ReadPIO(void *data, u16 count);
	void ReadAsyncDMA(void *data, u16 count);
	void ReadSyncDMA(void *data, u16 count);

	virtual	void InitHW();
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern void SPI_Init();
extern bool SPI_AddRequest(DSCSPI *d);
extern bool SPI_Update();
//extern i32	Get_FRAM_SPI_SessionsAdr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // SPI_H__21_04_2022__11_18
