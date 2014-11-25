ROHC=/usr/local/
ROHCLIB=${ROHC}lib/librohc_comp.a ${ROHC}lib/librohc_decomp.a ${ROHC}lib/librohc.a ${ROHC}lib/librohc_common.a
ROHCINC=-I${ROHC}/include/
CC=gcc
CFLAGS=-Wall -I../libowfat/ ${ROHCINC} -g
LDLIBS=../libowfat/libowfat.a -g ${ROHCLIB}
OBJ=tun_alloc.o nic.o ezrohc.o

all: noq push

noq: ${OBJ}

push:

clean:
	rm noq *.o



