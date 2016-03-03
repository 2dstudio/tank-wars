#ifndef _HX8357_1284_H_
#define _HX8357_1284_H_
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include "1284SPI.h"
#include "Adafruit_HX8357.h"

#define width  320
#define height 480
#define RSPORT PORTB
#define rspin 0
#define RS 0			// RS/DC PIN

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color);

void displayInit ()
{
	SoftwareReset();
	writecommand(HX8357_SWRESET);

	// setextc
	writecommand(HX8357D_SETC);
	writedata(0xFF);
	writedata(0x83);
	writedata(0x57);
	_delay_ms(300);
	// setRGB which also enables SDO
	writecommand(HX8357_SETRGB);
	writedata(0x80);  //enable SDO pin!
	//    writedata(0x00);  //disable SDO pin!
	writedata(0x0);
	writedata(0x06);
	writedata(0x06);

	writecommand(HX8357D_SETCOM);
	writedata(0x25);  // -1.52V
	
	writecommand(HX8357_SETOSC);
	writedata(0x68);  // Normal mode 70Hz, Idle mode 55 Hz
	
	writecommand(HX8357_SETPANEL); //Set Panel
	writedata(0x05);  // BGR, Gate direction swapped
	
	writecommand(HX8357_SETPWR1);
	writedata(0x00);  // Not deep standby
	writedata(0x15);  //BT
	writedata(0x1C);  //VSPR
	writedata(0x1C);  //VSNR
	writedata(0x83);  //AP
	writedata(0xAA);  //FS
	
	writecommand(HX8357D_SETSTBA);
	writedata(0x50);  //OPON normal
	writedata(0x50);  //OPON idle
	writedata(0x01);  //STBA
	writedata(0x3C);  //STBA
	writedata(0x1E);  //STBA
	writedata(0x08);  //GEN
	
	writecommand(HX8357D_SETCYC);
	writedata(0x02);  //NW 0x02
	writedata(0x40);  //RTN
	writedata(0x00);  //DIV
	writedata(0x2A);  //DUM
	writedata(0x2A);  //DUM
	writedata(0x0D);  //GDON
	writedata(0x78);  //GDOFF
	
	writecommand(HX8357D_SETGAMMA);
	writedata(0x02);
	writedata(0x0A);
	writedata(0x11);
	writedata(0x1d);
	writedata(0x23);
	writedata(0x35);
	writedata(0x41);
	writedata(0x4b);
	writedata(0x4b);
	writedata(0x42);
	writedata(0x3A);
	writedata(0x27);
	writedata(0x1B);
	writedata(0x08);
	writedata(0x09);
	writedata(0x03);
	writedata(0x02);
	writedata(0x0A);
	writedata(0x11);
	writedata(0x1d);
	writedata(0x23);
	writedata(0x35);
	writedata(0x41);
	writedata(0x4b);
	writedata(0x4b);
	writedata(0x42);
	writedata(0x3A);
	writedata(0x27);
	writedata(0x1B);
	writedata(0x08);
	writedata(0x09);
	writedata(0x03);
	writedata(0x00);
	writedata(0x01);
	
	writecommand(HX8357_COLMOD);
	writedata(0x55);  // 16 bit
	
	writecommand(HX8357_MADCTL);
	writedata(0xC0);
	
	writecommand(HX8357_TEON);  // TE off
	writedata(0x00);
	
	writecommand(HX8357_TEARLINE);  // tear line
	writedata(0x00);
	writedata(0x02);
	
	writecommand(HX8357_SLPOUT); //Exit Sleep
	_delay_ms(150);
	
	writecommand(HX8357_DISPON);  // display on
	_delay_ms(50);
	_delay_ms(100);
	writecommand(HX8357B_NORON);  // normal display on
	_delay_ms(10);
	
}

void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

	writecommand(HX8357_CASET); // Column addr set
	writedata(x0 >> 8);
	writedata(x0 & 0xFF);     // XSTART
	writedata(x1 >> 8);
	writedata(x1 & 0xFF);     // XEND

	writecommand(HX8357_PASET); // Row addr set
	writedata(y0>>8);
	writedata(y0);     // YSTART
	writedata(y1>>8);
	writedata(y1);     // YEND

	writecommand(HX8357_RAMWR); // write to RAM
}

inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void fillScreen(uint16_t color) {
	
		fillRect(0, 0, width, height, color);
	
}

void drawPixel(int16_t x, int16_t y, uint16_t color) {
	//if((x < 0) ||(x >= width) || (y < 0) || (y >= height)) return;

	setAddrWindow(x,y,x+1,y+1);
	
	//RSPORT |= (1 << RS);
	RSPORT |= (1 << RS);

	spiwrite(color >> 8);
	spiwrite(color);

}

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	// Rudimentary clipping
	if((x >= width) || (y >= height)) return;
	if((y+h-1) >= height) h = height-y;
	setAddrWindow(x, y, x, y+h-1);

	uint8_t hi = color >> 8, lo = color;
	RSPORT |= (1 << RS);

	while (h--) {
		spiwrite(hi);
		spiwrite(lo);
	}
	RSPORT |= (0 << RS);
}

void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {

    // Rudimentary clipping
    if((x >= width) || (y >= height)) return;
    if((x+w-1) >= width)  w = width-x;
    setAddrWindow(x, y, x+w-1, y);

    uint8_t hi = color >> 8, lo = color;
    RSPORT |= (1 << RS);
    while (w--) {
        spiwrite(hi);
        spiwrite(lo);
    }
}

// fill a rectangle
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color) {
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= width) || (y >= height)) return;
	if((x + w - 1) >= width)  w = width  - x;
	if((y + h - 1) >= height) h = height - y;

	setAddrWindow(x, y, x+w-1, y+h-1);

	uint8_t hi = color >> 8, lo = color;
	RSPORT |= (1 << RS);

	for(y=h; y>0; y--) {
		for(x=w; x>0; x--) {
			spiwrite(hi);
			spiwrite(lo);
		}
	}
}
// draw a character
void drawChar(uint8_t x, uint8_t y, char c, uint16_t color, uint8_t size) {
	for (uint8_t i =0; i<5; i++ ) {
		uint8_t line = pgm_read_byte(font+(c*5)+i);
		for (uint8_t j = 0; j<8; j++) {
			if (line & 0x1) {
				if (size == 1) // default size
				drawPixel(x+i, y+j, color);
				else {  // big size
					fillRect(x+i*size, y+j*size, size, size, color);
				}
			}
			line >>= 1;
		}
	}
}

void drawString(uint8_t x, uint8_t y, char *c, uint16_t color, uint8_t size) {
	while (c[0] != 0) {
		drawChar(x, y, c[0], color, size);
		x += size*6;
		c++;
		if (x + 5 >= width) {
			y += 10;
			x = 0;
		}
	}
}

void invertDisplay (bool i) 
{
	writecommand(i ? HX8357_INVON : HX8357_INVOFF);
}

void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	drawPixel(x0  , y0+r, color);
	drawPixel(x0  , y0-r, color);
	drawPixel(x0+r, y0  , color);
	drawPixel(x0-r, y0  , color);

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		drawPixel(x0 + x, y0 + y, color);
		drawPixel(x0 - x, y0 + y, color);
		drawPixel(x0 + x, y0 - y, color);
		drawPixel(x0 - x, y0 - y, color);
		drawPixel(x0 + y, y0 + x, color);
		drawPixel(x0 - y, y0 + x, color);
		drawPixel(x0 + y, y0 - x, color);
		drawPixel(x0 - y, y0 - x, color);
	}
}

void drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f     += ddF_y;
		}
		x++;
		ddF_x += 2;
		f     += ddF_x;
		if (cornername & 0x4) {
			drawPixel(x0 + x, y0 + y, color);
			drawPixel(x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) {
			drawPixel(x0 + x, y0 - y, color);
			drawPixel(x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			drawPixel(x0 - y, y0 + x, color);
			drawPixel(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			drawPixel(x0 - y, y0 - x, color);
			drawPixel(x0 - x, y0 - y, color);
		}
	}
}

void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) {

	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f     += ddF_y;
		}
		x++;
		ddF_x += 2;
		f     += ddF_x;

		if (cornername & 0x1) {
			drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
			drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
		}
		if (cornername & 0x2) {
			drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
			drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
		}
	}
}

void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	drawFastVLine(x0, y0-r, 2*r+1, color);
	fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Draw a rectangle
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	drawFastHLine(x, y, w, color);
	drawFastHLine(x, y+h-1, w, color);
	drawFastVLine(x, y, h, color);
	drawFastVLine(x+w-1, y, h, color);
}

void fillRect_GFX(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	// Update in subclasses if desired!
	for (int16_t i=x; i<x+w; i++) {
		drawFastVLine(i, y, h, color);
	}
}

// Draw a rounded rectangle
void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
	// smarter version
	drawFastHLine(x+r  , y    , w-2*r, color); // Top
	drawFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
	drawFastVLine(x    , y+r  , h-2*r, color); // Left
	drawFastVLine(x+w-1, y+r  , h-2*r, color); // Right
	// draw four corners
	drawCircleHelper(x+r    , y+r    , r, 1, color);
	drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
	drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
	drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}

// Fill a rounded rectangle
void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
	// smarter version
	fillRect(x+r, y, w-2*r, h, color);

	// draw four corners
	fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
	fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		_swap_int16_t(x0, y0);
		_swap_int16_t(x1, y1);
	}

	if (x0 > x1) {
		_swap_int16_t(x0, x1);
		_swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
		} else {
		ystep = -1;
	}

	for (; x0<=x1; x0++) {
		if (steep) {
			drawPixel(y0, x0, color);
			} else {
			drawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}
// Draw a triangle
void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {

  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
  }
  if (y1 > y2) {
    _swap_int16_t(y2, y1); _swap_int16_t(x2, x1);
  }
  if (y0 > y1) {
    _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
  int32_t
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) _swap_int16_t(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) _swap_int16_t(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }
}

void DisplayRun ()
{
	SPI_MasterInit();                  //Initialize SPI Master
	displayInit();	
}

#endif