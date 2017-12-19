CFLAGS=-c -Wall -O2
LIBS = -lm -lpthread

all: liboled.a

liboled.a: fonts.o aio_oled.o
	ar -rc liboled.a aio_oled.o fonts.o ;\
	sudo cp liboled.a /usr/local/lib ;\
	sudo cp aio_oled.h /usr/local/include

aio_oled.o: aio_oled.c aio_oled.h
	$(CC) $(CFLAGS) aio_oled.c

fonts.o: fonts.c
	$(CC) $(CFLAGS) fonts.c

clean:
	rm *.o liboled.a
