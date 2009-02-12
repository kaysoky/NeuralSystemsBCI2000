##############################################################################
#
# GNU'ish Makefile
#
# $Log: Makefile,v $
# Revision 1.4  2008/07/09 10:39:44  roboos
# cleaned up, ensure that all dependencies are matched
#
# Revision 1.3  2008/07/09 10:09:40  roboos
# renamed the various demo applications into demo_xxx
#
# Revision 1.2  2008/07/08 20:24:43  roboos
# added event example
#
# Revision 1.1  2008/07/08 18:52:30  roboos
# initial version, helps in compiling test and demo applications
#
##############################################################################

CC		 = gcc
BINDIR	 = .
INCPATH  = -I../src
LIBPATH  = -L../src
LIBS 	 = -lbuffer -lm -lpthread
CFLAGS	 = $(INCPATH) -Wunused -pedantic -O2
CXXFLAGS = $(INCPATH) -Wunused -pedantic -O2

##############################################################################

all: test demo

test: test_gethdr test_getdat test_getevt test_flushhdr test_flushdat test_flushevt

demo: demo_sinewave demo_event demo_buffer demo_combined

demo_combined: demo_combined.o sinewave.o
	$(CC) $(LIBPATH) -o $(BINDIR)/$@ $^ $(LIBS)

demo_sinewave: demo_sinewave.o sinewave.o
	$(CC) $(LIBPATH) -o $(BINDIR)/$@ $^ $(LIBS)

demo_event: demo_event.o event.o
	$(CC) $(LIBPATH) -o $(BINDIR)/$@ $^ $(LIBS)

demo_buffer: demo_buffer.o
	$(CC) $(LIBPATH) -o $(BINDIR)/$@ $^ $(LIBS)

test_gethdr: test_gethdr.o
	$(CC) $(LIBPATH) -o $(BINDIR)/$@ $^ $(LIBS)

test_getdat: test_getdat.o
	$(CC) $(LIBPATH) -o $(BINDIR)/$@ $^ $(LIBS)

test_getevt: test_getevt.o
	$(CC) $(LIBPATH) -o $(BINDIR)/$@ $^ $(LIBS)

test_flushhdr: test_flushhdr.o
	$(CC) $(LIBPATH) -o $(BINDIR)/$@ $^ $(LIBS)

test_flushdat: test_flushdat.o
	$(CC) $(LIBPATH) -o $(BINDIR)/$@ $^ $(LIBS)

test_flushevt: test_flushevt.o
	$(CC) $(LIBPATH) -o $(BINDIR)/$@ $^ $(LIBS)

%.o: %.c message.h buffer.h
	$(CC) $(CFLAGS) $(INCPATH) -c $*.c

clean:
	rm -f core *.o *.obj *.a *~

distclean:
	rm test_gethdr test_getdat test_getevt test_flushhdr test_flushdat test_flushevt
	rm demo_sinewave demo_event demo_buffer demo_combined

