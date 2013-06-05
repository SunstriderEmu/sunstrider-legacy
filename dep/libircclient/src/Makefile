# $Id: Makefile.in 105 2012-01-22 22:13:17Z gyunaev $
CC = gcc
CFLAGS = -Wall -DIN_BUILDING_LIBIRC -g -O2 -O3 -fpic -DENABLE_THREADS -D_REENTRANT -DENABLE_SSL
AR=ar cr
RANLIB=ranlib
INCLUDES=-I../include
DESTDIR=

OBJS = libircclient.o

all:	shared

static:	libircclient.a
shared: libircclient.so
shared_mingw: libircclient.dll

install: lib
	-mkdir -p $(DESTDIR)/home/mathman/windrunner/build/src/wrchat/libircclient-prefix/include
	-mkdir -p $(DESTDIR)/home/mathman/windrunner/build/src/wrchat/libircclient-prefix/lib
	cp ../include/libircclient.h $(DESTDIR)/home/mathman/windrunner/build/src/wrchat/libircclient-prefix/include/libircclient.h
	cp ../include/libirc_errors.h $(DESTDIR)/home/mathman/windrunner/build/src/wrchat/libircclient-prefix/include/libirc_errors.h
	cp ../include/libirc_events.h  $(DESTDIR)/home/mathman/windrunner/build/src/wrchat/libircclient-prefix/include/libirc_events.h 
	cp ../include/libirc_options.h  $(DESTDIR)/home/mathman/windrunner/build/src/wrchat/libircclient-prefix/include/libirc_options.h 
	cp ../include/libirc_rfcnumeric.h  $(DESTDIR)/home/mathman/windrunner/build/src/wrchat/libircclient-prefix/include/libirc_rfcnumeric.h 
	cp libircclient.so $(DESTDIR)/home/mathman/windrunner/build/src/wrchat/libircclient-prefix/lib/
	cp libircclient.a $(DESTDIR)/home/mathman/windrunner/build/src/wrchat/libircclient-prefix/lib/

libircclient.so: libircclient.o
	$(CC) -shared -s -o libircclient.so libircclient.o -lssl -lcrypto  -lnsl

libircclient.dll: libircclient.o
	$(CC) -shared -s -o libircclient.dll -Wl,-out-implib,libircclient.lib libircclient.o libircclient.def -lkernel32 -lwsock32 -lssl -lcrypto  -lnsl

libircclient.a: libircclient.o
	$(AR) libircclient.a libircclient.o
	$(RANLIB) libircclient.a

clean:
	rm -f libircclient.a libircclient.o

distclean: clean
	-rm -f Makefile

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
