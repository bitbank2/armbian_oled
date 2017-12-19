#ifndef AIO_OLED_H
#define AIO_OLED_H
//
// ArmbianIO OLED
// Library for accessing SSD1306 OLED displays
// Written by Larry Bank (bitbank@pobox.com)
// Copyright (c) 2017 BitBank Software, Inc.
// Project started 1/15/2017
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

// Initialize the OLED library
// Optionally enable inverted or flipped mode
// returns 0 for success, 1 for failure
//
int oledInit(int iChannel, int iAddr, int bFlip, int bInvert, int bSPI, int iDC, int iReset);

// Turns off the display and closes any open handles
void oledShutdown(void);

// Fills the display with the byte pattern
int oledFill(unsigned char ucPattern);

// Write a text string to the display at x (column 0-127) and y (row 0-7)
// bLarge = 0 - 8x8 font, bLarge = 1 - 16x24 font
int oledWriteString(int x, int y, char *szText, int bLarge);

// Sets a pixel to On (1) or Off (0)
// Coordinate system is pixels, not text rows (0-127, 0-63)
int oledSetPixel(int x, int y, unsigned char ucPixel);

// Sets the contrast (brightness) level of the display
// Valid values are between 0=off and 255=max brightness
int oledSetContrast(unsigned char ucContrast);
#endif // AIO_OLED_H
