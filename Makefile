ROHC=/usr/local/
ROHCLIB=${ROHC}lib/librohc_comp.a ${ROHC}lib/librohc_decomp.a ${ROHC}lib/librohc.a ${ROHC}lib/librohc_common.a
ROHCINC=-I${ROHC}/include/
CC=gcc
#CFLAGS=-b -Wall -Wunsupported -Werror -Wwrite-strings -I../libowfat/ ${ROHCINC} -g
#CFLAGS=-b -Wall -Wwrite-strings -I../libowfat/ ${ROHCINC} -g
CFLAGS=-static -Wall -Wwrite-strings ${ROHCINC} -g
LDLIBS=-g ${ROHCLIB}
OBJ=tun_alloc.o nic.o ezrohc.o sck.o

all: noq push

noq: ${OBJ}

push:

clean:
	rm noq *.o



