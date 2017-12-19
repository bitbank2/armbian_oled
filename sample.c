//
// OLED test program
// Written by Larry Bank
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
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "aio_oled.h"

int main(int argc, char *argv[])
{
int i;

//	i = oledInit(0, 0, 0, 0, 1, 37, 35); // SPI init 
	i = oledInit(0, 0x3c, 0, 0, 0, 0, 0); // I2C init
	if (i == 0)
	{
		oledFill(0); // fill with black
		oledWriteString(0,0,"OLED 96 Library!",0);
		oledWriteString(0,1,"<Line 1 line 1>",0);
		oledWriteString(0,2,"<Line 2 line 2>",0);
		oledWriteString(0,3,"<Line 3 line 3>",0);
		oledWriteString(0,4,"<Line 4 line 4>",0);
		oledWriteString(0,5,"<Line 5 line 5>",0);
		oledWriteString(0,6,"<Line 6 line 6>",0);
		oledWriteString(0,7,"<Line 7 line 7>",0);
		for (i=0; i<64; i++)
		{
			oledSetPixel(i, 16+i, 1);
			oledSetPixel(127-i, 16+i, 1);
		}
		getchar(); // wait for enter to quit
		oledShutdown();
	}
	else
	{
		printf("Failed to initialize the OLED\n");
	}
   return 0;
} /* main() */
