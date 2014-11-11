CFLAGS=-Wall -I../../lib/libowfat/ -g
LDLIBS=../../lib/libowfat/libowfat.a -g
OBJ=tun_alloc.o nic.o

all: noq push

noq: ${OBJ}

push:

clean:
	rm noq *.o

