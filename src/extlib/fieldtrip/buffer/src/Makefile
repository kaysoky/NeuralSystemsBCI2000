##############################################################################
#
# GNU'ish Makefile
#
# $Log: Makefile,v $
# Revision 1.18  2008/07/09 13:34:57  roboos
# compiling the buffer mex file on matlab75/64bt linux requires -fPIC
#
# Revision 1.17  2008/07/09 10:39:44  roboos
# cleaned up, ensure that all dependencies are matched
#
# Revision 1.16  2008/07/08 18:53:50  release
# *** empty log message ***
#
# Revision 1.15  2008/07/08 18:40:01  release
# cleanup, removed demo stuff, only keep core functions for libbuffer and libclient
#
##############################################################################

CC	 = gcc
INCPATH	 = -I.
LIBPATH  = -L.
BINDIR	 = .
LIBS 	 = -lbuffer -lm -lpthread
CFLAGS	 = $(INCPATH) -Wunused -pedantic -O2 -fPIC
CXXFLAGS = $(INCPATH) -Wunused -pedantic -O2

##############################################################################

all: libbuffer.a

libbuffer.a: tcpserver.o tcpsocket.o tcprequest.o clientrequest.o dmarequest.o cleanup.o util.o printstruct.o swapbytes.o property.o
	 ar rv $@ $^

libclient.a: tcprequest.o util.o
	 ar rv $@ $^

%.o: %.c buffer.h message.h swapbytes.h socket_includes.h unix_includes.h
	$(CC) $(CFLAGS) -c $*.c

clean:
	rm -f core *.o *.obj *.a *~

