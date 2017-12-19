AIO OLED<br>

The ArmbianIO OLED library is for working with SSD1306 OLED displays.
It supports both the I2C and SPI interfaces and uses the ArmbianIO library
to simplify access to I2C, SPI and GPIO pins.
The idea is to provide a simple interface for C programmers to make use of 
those low-cost OLED displays without having to read the data sheet or figure
out I2C/SPI programming.<br>

Written by Larry Bank<br>
Project started 1/15/2017<br>
bitbank@pobox.com<br>

Features:<br>
--------<br>
- Efficient individual pixel drawing<br>
- Small and large text<br>
- Invert and flip 180 options<br>
- Brightness control<br>

The ArmbianIO library can be found here:<br>
https://github.com/bitbank2/ArmbianIO<br>

Building the library:<br>
---------------------<br>
make<br>

This will copy the liboled.a file to /usr/local/lib and aio_oled.h to
/usr/local/include<br>

Now you can build the included demo program:<br>
--------------------------------------------<br>
make -f make_sample<br>
sudo ./sample<br>

