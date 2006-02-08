####################################################################
# File:        GNUmakefile
# Date:        Jan 10, 2006
# Author:      juergen.mellinger@uni-tuebingen.de
# Description: GCC makefile for BCI2000 command line tools.
#              You can create any BCI2000 filter as a
#              command line tool by entering "make myfilter.exe"
#              at a windows command prompt in the directory
#              containing this Makefile, provided there exists
#              a file "myfilter.cpp" in one of the paths listed
#              in the SIGPROC variable.
#              Aside from testing purposes, this may be useful
#              for offline data analysis.
####################################################################

AND = &&
OR  = ||

TARGETS = bci_dat2stream \
          bci_prm2stream \
          bci_stream2prm \
          bci_stream2asc \
          bci_stream2table \
          bci_stream2mat \
          CalibrationFilter \
          classfilter \
          Normalfilter \
          SpatialFilter \
          SWFilter \
          P3TemporalFilter \
          Normalizer \
          ConditionalIntegrator \
          ComplexDemodulator \
          StateTransform \
          UPeakDetector \
          FFTFilter \
          TransmissionFilter \
#          StatFilter \
#          ARFilter \
#          FIRFilter \

BCIOBJ  = bci_stubs.o \
          UParameter.o \
          EncodedString.o \
          ParamRef.o \
          UGenericSignal.o \
          UEnvironment.o \
          UGenericFilter.o \
          UGenericVisualization.o \
          UBCIError.o \
          UState.o \
          UStatus.o \
          USysCommand.o \
          MessageHandler.o \
          MeasurementUnits.o \
          Expression.o \
          ExpressionParser.o \
          GETMEM.o \
          getfir.o \
          FFTLibWrap.o

STATOBJ = bci_filtertool.o \
          StatFilter.o \
          Statistics.o \
          Normalfilter.o \
          classfilter.o \

BCIROOT = ../..
SRCROOT = ${BCIROOT}/EEGSource
SIGROOT = ${BCIROOT}/SignalProcessing
SIGPROC = ${SIGROOT}:${SIGROOT}/AR:${SIGROOT}/FIR:${SIGROOT}/P3:${SIGROOT}/PeakDetector:${SIGROOT}/SW:${SIGROOT}/fftlib:${SIGROOT}/misc
PATHCPP = ${BCIROOT}/shared:${BCIROOT}/shared/Expression:${SIGPROC}:${SRCROOT}
INCLUDEPATH = -I${BCIROOT} -I${BCIROOT}/shared
LDLIBS = -lfftw3

DEFINES = -D_DEBUG -DBCI_TOOL -DNO_STRICT -D_NO_VCL -DSIGNAL_BACK_COMPAT -D_GNU_SOURCE
CFLAGS = -include gccprefix.h

CPP = g++ ${CFLAGS} ${DEFINES} ${INCLUDEPATH}
LD  = g++ 
AR  = ar -cr
RM  = rm

vpath %.cpp ${PATHCPP}

all: ${TARGETS}

.cpp:
	${MAKE} bci_tool.lib
	${MAKE} bci_tool.o
	${MAKE} bci_filtertool.o
	${CPP} -Wl,--allow-multiple-definition $< bci_filtertool.o bci_tool.o bci_tool.lib \
	  $(LOADLIBES) $(LDLIBS) -o $@

.o:
	${MAKE} bci_tool.lib
	${MAKE} bci_tool.o
	${MAKE} bci_filtertool.o
	${CPP} -Wl,--allow-multiple-definition $< bci_filtertool.o bci_tool.o bci_tool.lib \
	  $(LOADLIBES) $(LDLIBS) -o $@

.cpp.o: 
	${CPP} -c -o $@ $<

bci_tool.lib: ${BCIOBJ}
	${AR} $@ $^

StatFilter.lib: ${STATOBJ}
	${AR} $@ $^

StatFilter.exe: StatFilter.lib bci_tool.lib bci_tool.o
	${CPP} -Wl,--allow-multiple-definition $^ $(LOADLIBES) $(LDLIBS) -o $@
	

clean:
	-${RM} *.o *.lib /*.BAK

distclean: clean
	-${RM} *.exe

