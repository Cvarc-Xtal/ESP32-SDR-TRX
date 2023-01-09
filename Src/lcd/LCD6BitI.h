/////////////////////////////////////
//Дисплей TFTLCD24BIT https://datasheetspdf.com/pdf-file/746163/LG/LB043WQ2-TD01/1
//синхронизация через DE
//1,2    - GND
//3,4    - VCC (3.3v)
//5..12  - RED          
//13..20 - GREEN
//21..28 - BLUE
//29     - GND
//30     - CLOCK
//31     - +3.3v (дисплей всегда ON)
//34     - DE   (data enable - тип синхронизации без hsync/vsync, DE-only)
//35     - +3.3v (PWSEL)
//36     - GND
//37     - Y2 TOP //touch
//38     - X2 LEFT //touch
//39     - Y1 BOTTOM //touch
//40     - X1 RIGHT //touch
//41     - GND
//42     - LED-(подсветка, 28 вольт с огр.резистором 15ом)
//43     - LED+

//ОПРЕДЕЛЕНИЯ
//LCD6BitI tft;
//const Mode VGA::MODE480x272(0, 48,  0, 480,  0, 14, 0, 272, 1, 9200000, 1, 1); //для LB043WQ2-TD01 sync DE only
//
//инициализация
// Mode myMode = tft.MODE480x272;//I2S1 - для дисплея TFT24 в режим 6-бит цвет(R2G2B2)
//tft.init(myMode, REDPIN,REDPIN1,GREENPIN,GREENPIN1,BLUEPIN,BLUEPIN1,HSYNCPIN,DEPIN,CLOCKPIN);
//
#pragma once
#include <ESP32Lib.h>

i2s_dev_t *i2sDev[] = {&I2S0, &I2S1};

class LCD6BitI : public VGA, public GraphicsR2G2B2A2
{
  public:
	LCD6BitI()	//8 bit based modes only work with I2S1
		: VGA(1)
	{
		interruptStaticChild = &LCD6BitI::interrupt;
	}
 //////////////////////////////////////////
  static const Mode MODE480x272;
 /////////////////////////////////////////

	bool init(const Mode &mode,
			  const int R0Pin, const int R1Pin,
			  const int G0Pin, const int G1Pin,
			  const int B0Pin, const int B1Pin,
			  const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		int pinMap[8] = {
			R0Pin, R1Pin,
			G0Pin, G1Pin,
			B0Pin, B1Pin,
			hsyncPin, vsyncPin
		};
	 bool ret = VGA::init(mode, pinMap, 8, clockPin);
   ///////////////////////////////////
   div_corrections();
   return ret;
   ///////////////////////////////////
	}

	bool init(const Mode &mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		int pinMap[8];
		for (int i = 0; i < 2; i++)
		{
			pinMap[i] = redPins[i];
			pinMap[i + 2] = greenPins[i];
			pinMap[i + 4] = bluePins[i];
		}
		pinMap[6] = hsyncPin;
		pinMap[7] = vsyncPin;			
		bool ret = VGA::init(mode, pinMap, 8, clockPin);
   ///////////////////////////////////
   div_corrections();
   return ret;
   ///////////////////////////////////
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		int pins[8];
		pinConfig.fill6Bit(pins);
		bool ret = VGA::init(mode, pins, 8, pinConfig.clock);
   ///////////////////////////////////
   div_corrections();
   return ret;
   ///////////////////////////////////
	}
///////////////////////////////////////////
  
  void div_corrections(){
     volatile i2s_dev_t &i2s = *i2sDev[i2sIndex];
     i2sStop();
     i2s.clkm_conf.clka_en = 0;      //источник тактирования APLL_SCLK = off, PLL_D2_CLK = on
     i2s.clkm_conf.clkm_div_num = 8;
     i2s.clkm_conf.clkm_div_a = 1;
     i2s.clkm_conf.clkm_div_b = 1;
     i2s.sample_rate_conf.tx_bck_div_num = 2;
     startTX();
  }
/////////////////////////////////////////
	virtual void initSyncBits()
	{
		hsyncBitI = mode.hSyncPolarity ? 0x40 : 0;
		vsyncBitI = mode.vSyncPolarity ? 0x80 : 0;
		hsyncBit = hsyncBitI ^ 0x40;
		vsyncBit = vsyncBitI ^ 0x80;
	}

	virtual long syncBits(bool hSync, bool vSync)
	{
		return ((hSync ? hsyncBit : hsyncBitI) | (vSync ? vsyncBit : vsyncBitI)) * 0x1010101;
	}

	virtual int bytesPerSample() const
	{
		return 1;
	}

	virtual float pixelAspect() const
	{
		return 1;
	}

	virtual void propagateResolution(const int xres, const int yres)
	{
		setResolution(xres, yres);
	}

	virtual void show(bool vSync = false)
	{
		if (!frameBufferCount)
			return;
		if (vSync)
		{
			vSyncPassed = false;
			while (!vSyncPassed)
				delay(0);
		}
		Graphics::show(vSync);
	}

  protected:
	bool useInterrupt()
	{ 
		return true; 
	};

	static void interrupt(void *arg);

	static void interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits, void *arg);
};


void IRAM_ATTR LCD6BitI::interrupt(void *arg)
{
	LCD6BitI * staticthis = (LCD6BitI *)arg;
	
	unsigned long *signal = (unsigned long *)staticthis->dmaBufferDescriptors[staticthis->dmaBufferDescriptorActive].buffer();
	unsigned long *pixels = &((unsigned long *)staticthis->dmaBufferDescriptors[staticthis->dmaBufferDescriptorActive].buffer())[(staticthis->mode.hSync + staticthis->mode.hBack) / 4];
	unsigned long base, baseh;
	if (staticthis->currentLine >= staticthis->mode.vFront && staticthis->currentLine < staticthis->mode.vFront + staticthis->mode.vSync)
	{
		baseh = (staticthis->hsyncBit | staticthis->vsyncBit) * 0x1010101;
		base = (staticthis->hsyncBitI | staticthis->vsyncBit) * 0x1010101;
	}
	else
	{
		baseh = (staticthis->hsyncBit | staticthis->vsyncBit) * 0x1010101;
		base = (staticthis->hsyncBitI | staticthis->vsyncBitI) * 0x1010101;
	}
	for (int i = 0; i < staticthis->mode.hSync / 4; i++)
		signal[i] = baseh;
	for (int i = staticthis->mode.hSync / 4; i < (staticthis->mode.hSync + staticthis->mode.hBack) / 4; i++)
		signal[i] = base;

	int y = (staticthis->currentLine - staticthis->mode.vFront - staticthis->mode.vSync - staticthis->mode.vBack) / staticthis->mode.vDiv;
	if (y >= 0 && y < staticthis->mode.vRes)
		staticthis->interruptPixelLine(y, pixels, base, arg);
	else
		for (int i = 0; i < staticthis->mode.hRes / 4; i++)
		{
			pixels[i] = base;
		}
	for (int i = 0; i < staticthis->mode.hFront / 4; i++)
		signal[i + (staticthis->mode.hSync + staticthis->mode.hBack + staticthis->mode.hRes) / 4] = base;
	staticthis->currentLine = (staticthis->currentLine + 1) % staticthis->totalLines;
	staticthis->dmaBufferDescriptorActive = (staticthis->dmaBufferDescriptorActive + 1) % staticthis->dmaBufferDescriptorCount;
	if (staticthis->currentLine == 0)
		staticthis->vSyncPassed = true;
}

void IRAM_ATTR LCD6BitI::interruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits, void *arg)
{
	LCD6BitI * staticthis = (LCD6BitI *)arg;
	unsigned char *line = staticthis->frontBuffer[y];
	int j = 0;
	for (int i = 0; i < staticthis->mode.hRes / 4; i++)
	{
		int p0 = (line[j++]) & 63;
		int p1 = (line[j++]) & 63;
		int p2 = (line[j++]) & 63;
		int p3 = (line[j++]) & 63;
		pixels[i] = syncBits | (p2 << 0) | (p3 << 8) | (p0 << 16) | (p1 << 24);
	}
}
