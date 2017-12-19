//
// OLED SSD1306 using either the I2C or SPI interfaces
// Written by Larry Bank (bitbank@pobox.com)
// Project started 1/15/2017
//
// The SSD1306 controller is used for a few display sizes. I've tested 128x64
// and 64x48 displays. The 64x48 behaves as if it's 128x64, but only displays
// the center 64 columns and bottom 48 lines.
//
// The OLED controller needs to know if data is a command or pixel data.
// This is done by a prefix byte (0x00=command, 0x40=data) in I2C mode or a
// dedicated pin (D/C - 0=command, 1=data) in SPI mode. When in I2C mode,
// the writes (through a file handle) can be single or multiple bytes.
// The write mode stays in effect throughout each call to write()
// The OLED controller is set to "page mode". This divides the display
// into 8 128x8 "pages" or strips. Each data write advances the output
// automatically to the next address. The bytes are arranged such that the LSB
// is the topmost pixel and the MSB is the bottom.
// The font data comes from another source and must be rotated 90 degrees
// (at init time) to match the orientation of the bits on the display memory.
// A copy of the display memory is maintained by this code so that single pixel
// writes can occur without having to read from the display controller.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <armbianio.h>

extern unsigned char ucFont[];
static int iScreenOffset; // current write offset of screen data
static unsigned char ucScreen[1024]; // local copy of the image buffer
static int fHandle = -1;
static int bSPIMode; // boolean indicating I2C or SPI data mode
static int iDCPin, iResetPin; // header pin numbers for these inputs
static void RotateFont90(void);

//
// Write a byte sequence which should be interpreted as commands for the
// OLED controller
//
static int WriteCommand(unsigned char *pData, int iLen)
{
int rc;
	if (bSPIMode)
	{
		AIOWriteGPIO(iDCPin, 0); // set command mode
		rc = AIOWriteSPI(fHandle, pData, iLen);
	}
	else // I2C
	{
		rc = AIOWriteI2C(fHandle, 0x00, pData, iLen); // 'register' 0 tells the OLED that it's a command sequence
	}
	return rc;
} /* WriteCommand() */

//
// Write a byte sequence which should be interpreted as pixel data
//
static int WriteData(unsigned char *pData, int iLen)
{
int rc;
	if (bSPIMode)
	{
		AIOWriteGPIO(iDCPin, 1); // set data mode
		rc = AIOWriteSPI(fHandle, pData, iLen);
	}
	else // I2C
	{
		rc = AIOWriteI2C(fHandle, 0x40, pData, iLen); // 'register' 0x40 tells the OLED that it's a data sequence
	}
        memcpy(&ucScreen[iScreenOffset], pData, iLen);
        iScreenOffset += iLen;
	return rc;
} /* WriteData() */

//
// Opens a file system handle to the I2C device
// Initializes the OLED controller into "page mode"
// Prepares the font data for the orientation of the display
// Returns 0 for success, 1 for failure
// In SPI mode, the iAddr parameter is ignored
// in I2C mode, the iDC and iReset parameters are ignored
//
int oledInit(int iChannel, int iAddr, int bFlip, int bInvert, int bSPI, int iDC, int iReset)
{
unsigned char initbuf[]={0xae,0xa8,0x3f,0xd3,0x00,0x40,0xa1,0xc8,
                        0xda,0x12,0x81,0xff,0xa4,0xa6,0xd5,0x80,0x8d,0x14,
                        0xaf,0x20,0x02};

int rc;
unsigned char uc[4];

	if (!AIOInit())
	{
		fprintf(stderr, "Unable to initialize ArmbianIO library\n");
		return 1;
	}
        if (bSPI)
        { // initialize the GPIO pins and reset the device

		iDCPin = iDC;
		iResetPin = iReset;
		if (AIOAddGPIO(iDC, GPIO_OUT) != 1)
		{
			fprintf(stderr, "Error registering pin %d as an output\n", iDC);
			return 1;
		}
		if (AIOAddGPIO(iReset, GPIO_OUT) != 1)
		{
			fprintf(stderr, "Error registering pin %d as an output\n", iReset);
			return 1;
		}
		AIOWriteGPIO(iReset, 0);
		usleep(10000);
		AIOWriteGPIO(iReset, 1);
	}
	bSPIMode = bSPI;
	if (bSPI)
		fHandle = AIOOpenSPI(iChannel, 5000000); // 5Mhz is a reasonable speed for these little displays
	else
		fHandle = AIOOpenI2C(iChannel, iAddr);
	if (fHandle == -1)
		return 1; // failed, quit

	rc = WriteCommand(initbuf, sizeof(initbuf));
	RotateFont90(); // fix font orientation for OLED
	if (rc <= 0)
		return 1;
	if (bInvert)
	{
		uc[0] = 0xa7; // invert command
		rc = WriteCommand(uc, 1);
	}
	if (bFlip) // rotate display 180
	{
		uc[0] = 0xa0;
		rc = WriteCommand(uc, 1);
		uc[0] = 0xc0;
		rc = WriteCommand(uc, 1);
	}
	return 0;
} /* oledInit() */

//
// Sends a command to turn off the OLED display
// Closes the I2C file handle
//
void oledShutdown()
{
unsigned char uc;

	if (fHandle >= 0)
	{
		uc = 0xae;
		WriteCommand(&uc, 1); // turn off OLED
		if (bSPIMode)
			AIOCloseSPI(fHandle);
		else
			AIOCloseI2C(fHandle);
		fHandle = -1;
		AIOShutdown();
	}
}

//
// Set the 'brightness' of the OLED display
// valid values are 0 (off) to 255 (brightest)
//
int oledSetContrast(unsigned char ucContrast)
{
unsigned char ucTemp[2];

        if (fHandle < 0)
                return -1;

	ucTemp[0] = 0x81;
	ucTemp[1] = ucContrast;
	WriteCommand(ucTemp, 2);
	return 0;
} /* oledSetContrast() */

//
// Send commands to position the "cursor" to the given
// row and column
//
static void oledSetPosition(int x, int y)
{
unsigned char uc;

	uc = 0xb0 | y; // go to page Y
	WriteCommand(&uc, 1);
	uc = 0x00 | (x & 0xf); // lower col addr
	WriteCommand(&uc, 1);
	uc = 0x10 | ((x>>4) & 0xf); // upper col addr
	WriteCommand(&uc, 1);
	iScreenOffset = (y*128)+x;
}

//
// Set (or clear) an individual pixel
// The local copy of the frame buffer is used to avoid
// reading data from the display controller
//
int oledSetPixel(int x, int y, unsigned char ucColor)
{
int i;
unsigned char uc, ucOld;

	if (fHandle < 0)
		return -1;

	i = ((y >> 3) * 128) + x;
	if (i < 0 || i > 1023) // off the screen
		return -1;
	uc = ucOld = ucScreen[i];
	uc &= ~(0x1 << (y & 7));
	if (ucColor)
	{
		uc |= (0x1 << (y & 7));
	}
	if (uc != ucOld) // pixel changed
	{
		oledSetPosition(x, y>>3);
		WriteData(&uc, 1);
	}
	return 0;
} /* oledSetPixel() */

//
// Draw a string of small (8x8) or large (16x24) characters
// At the given col+row
//
int oledWriteString(int x, int y, char *szMsg, int bLarge)
{
int i, iLen;
unsigned char *s;

	if (fHandle < 0) return -1; // not initialized

	iLen = strlen(szMsg);
	if (bLarge) // draw 16x32 font
	{
		if (iLen+x > 8) iLen = 8-x;
		if (iLen < 0) return -1;
		x *= 16;
		for (i=0; i<iLen; i++)
		{
			s = &ucFont[9728 + (unsigned char)szMsg[i]*64];
			oledSetPosition(x+(i*16), y);
			WriteData(s, 16);
			oledSetPosition(x+(i*16), y+1);
			WriteData(s+16, 16);	
			oledSetPosition(x+(i*16), y+2);
			WriteData(s+32, 16);	
		}
	}
	else // draw 8x8 font
	{
		oledSetPosition(x*8, y);
		if (iLen + x > 16) iLen = 16 - x; // can't display it
		if (iLen < 0)return -1;

		for (i=0; i<iLen; i++)
		{
			s = &ucFont[(unsigned char)szMsg[i] * 8];
			WriteData(s, 8); // write character pattern
		}	
	}
	return 0;
} /* oledWriteString() */

//
// Fill the frame buffer with a byte pattern
// e.g. all off (0x00) or all on (0xff)
//
int oledFill(unsigned char ucData)
{
int y;
unsigned char temp[128];

	if (fHandle < 0) return -1; // not initialized

	memset(temp, ucData, 128);
	for (y=0; y<8; y++)
	{
		oledSetPosition(0,y); // set to (0,Y)
		WriteData(temp, 128); // fill with data byte
	} // for y
	return 0;
} /* oledFill() */

//
// Fix the orientation of the font image data
//
static void RotateFont90(void)
{
unsigned char ucTemp[64];
int i, j, x, y;
unsigned char c, c2, ucMask, *s, *d;

	// Rotate the 8x8 font
	for (i=0; i<256; i++) // fix 8x8 font by rotating it 90 deg clockwise
	{
		s = &ucFont[i*8];
		ucMask = 0x1;
		for (y=0; y<8; y++)
		{
			c = 0;
			for (x=0; x<8; x++)
			{
				c >>= 1;
				if (s[x] & ucMask) c |= 0x80;
			}
			ucMask <<= 1;
			ucTemp[7-y] = c;
		}
		memcpy(s, ucTemp, 8);
	}
	// Rotate the 16x32 font
	for (i=0; i<128; i++) // only 128 characters
	{
		for (j=0; j<4; j++)
		{
			s = &ucFont[9728 + 12 + (i*64) + (j*16)];
			d = &ucTemp[j*16];
			ucMask = 0x1;
			for (y=0; y<8; y++)
			{
				c = c2 = 0;
				for (x=0; x<8; x++)
				{
					c >>= 1;
					c2 >>= 1;
					if (s[(x*2)] & ucMask) c |= 0x80;
					if (s[(x*2)+1] & ucMask) c2 |= 0x80;
				}
				ucMask <<= 1;
				d[7-y] = c;
				d[15-y] = c2;
			} // for y
		} // for j
		memcpy(&ucFont[9728 + (i*64)], ucTemp, 64);
	} // for i
} /* RotateFont90() */
