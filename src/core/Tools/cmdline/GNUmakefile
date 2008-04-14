####################################################################
# $Id$
# Author:      juergen.mellinger@uni-tuebingen.de
# Description: GCC makefile for BCI2000 command line tools.
#              You can create any BCI2000 filter as a
#              command line tool by entering "make myfilter.exe"
#              at a windows command prompt in the directory
#              containing this Makefile, provided there exists
#              a file "myfilter.cpp" in one of the paths listed
#              in the PATHCPP variable.
#              Aside from testing purposes, this may be useful
#              for offline data analysis.
####################################################################

AND = &&
OR  = ||

TARGETS = bci_dat2stream \
          bci_prm2stream \
          bci_decimate \
          bci_stream2prm \
          bci_stream2asc \
          bci_stream2table \
          bci_stream2mat \
          LinearClassifier \
          Normalizer \
          SpatialFilter \
          ARFilter \
          P3TemporalFilter \
          ConditionalIntegrator \
          ComplexDemodulator \
          StateTransform \
          FFTFilter \
          TransmissionFilter \
#          MatlabFilter \


BCIOBJ  = bci_stubs.o \
          Param.o \
          ParamList.o \
          State.o \
          StateList.o \
          StateVector.o \
          EncodedString.o \
          LabelIndex.o \
          HierarchicalLabel.o \
          Brackets.o \
          ParamRef.o \
          GenericSignal.o \
          SignalProperties.o \
          SignalType.o \
          Environment.o \
          GenericFilter.o \
          GenericVisualization.o \
          BitmapImage.o \
          BCIError.o \
          BCIError_tool.o \
          Status.o \
          SysCommand.o \
          MessageHandler.o \
          MeasurementUnits.o \
          PhysicalUnit.o \
          ArithmeticExpression.o \
          Expression.o \
          ExpressionParser.o \
          ClassName.o \
          FFTLibWrap.o \
#          MatlabWrapper.o \

BCIROOT = ../../..
SRCROOT = ${BCIROOT}/shared/modules/signalsource
SIGROOT = ${BCIROOT}/shared/modules/signalprocessing
SIGPROC = ${BCIROOT}/core/SignalProcessing
EXTLIB =  ${BCIROOT}/extlib

INCLUDEPATH = -I${SRCROOT} -I${BCIROOT}/shared -I${BCIROOT}/shared/config -I${BCIROOT}/shared/accessors\
 -I${BCIROOT}/shared/bcistream -I${BCIROOT}/shared/fileio -I${BCIROOT}/shared/types\
 -I${BCIROOT}/shared/modules -I${BCIROOT}/shared/utils -I${BCIROOT}/shared/utils/Expression\
 -I${EXTLIB}/matlab -I${EXTLIB}/fftlib -I${EXTLIB}/math

PATHCPP = ${BCIROOT}:${BCIROOT}/shared:${BCIROOT}/shared/config:\
${BCIROOT}/shared/accessors:${BCIROOT}/shared/bcistream:\
${BCIROOT}/shared/fileio:${BCIROOT}/shared/types:\
${BCIROOT}/shared/modules:${BCIROOT}/shared/utils:\
${BCIROOT}/shared/utils/Expression:${SIGROOT}:\
${SIGROOT}/AR:${SIGROOT}/P3:${EXTLIB}/fftlib:\
${SIGROOT}/Matlab:${SIGROOT}/filters:${SRCROOT}:${SIGPROC}/Matlab

LDLIBS = -lfftw3

DEFINES = -D_DEBUG -DBCI_TOOL -DNO_STRICT -D_NO_VCL -D_GNU_SOURCE
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

clean:
	-${RM} *.o *.lib *.bak

distclean: clean
	-${RM} *.exe

