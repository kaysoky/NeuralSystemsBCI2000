####################################################################
# File:        Makefile
# Date:        Jul 18, 2003
# Author:      juergen.mellinger@uni-tuebingen.de
# Description: Makefile for BCI2000 command line tools.
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

TARGETS = obj\bci_tool.lib \
          bci_dat2stream.exe \
          bci_prm2stream.exe \
          bci_stream2asc.exe \
          CalibrationFilter.exe \
          ClassFilter.exe \
          Normalfilter.exe \
          SpatialFilter.exe \
          ARFilter.exe \
          FIRFilter.exe \
          FIRclassfilter.exe \
          P3TemporalFilter.exe \
          UPeakDetector.exe \
          SWFilter.exe \
          Segmenting.exe \
          SegmentFIR.exe \
          DropSegments.exe \
          SegmentPower.exe
          
BCIOBJ  = obj\bci_tool.obj \
          obj\bci_stubs.obj \
          obj\UParameter.obj \
          obj\UGenericSignal.obj \
          obj\UEnvironment.obj \
          obj\UGenericFilter.obj \
          obj\UBCIError.obj \
          obj\UState.obj \
          obj\UStatus.obj \
          obj\USysCommand.obj \
          obj\MessageHandler.obj \
          obj\MeasurementUnits.obj \
          obj\getmem.obj \
          obj\getfir.obj \
          obj\BoolExpression.obj

BCIROOT = ..\..
SIGROOT = ${BCIROOT}\SignalProcessing
SIGPROC = ${SIGROOT};${SIGROOT}\AR;${SIGROOT}\FIR;${SIGROOT}\P3;${SIGROOT}\PeakDetector;${SIGROOT}\SWSignalProcessing;${SIGROOT}\Analysis
PATHOBJ = .\obj
PATHCPP = .;${BCIROOT}\shared;${SIGPROC}
INCLUDEPATH = ${BCIROOT};${BCIROOT}\shared;${BCB}\include;${BCB}\include\vcl
LIBPATH = ${BCB}\lib\obj;${BCB}\lib

DEFINES = _DEBUG;BCI_TOOL;NO_STRICT;_NO_VCL;NO_VCL;SIGNAL_BACK_COMPAT
CFLAGS = -Od -Vx -Ve -X- -r- -a8 -b- -k -y -v -vi- -c -tW -tWM -w-par

CPP = ${CC} ${CFLAGS} -D${DEFINES} -I${INCLUDEPATH}
LD  = bcc32 -L${LIBPATH}
AR  = tlib /C /P128 /0 >nul # If this fails, try a larger value for the /P argument.
RM  = del 

.autodepend
.path.cpp=${PATHCPP}

{$(PATHOBJ)}.obj.exe:
    ${MAKE} obj\bci_tool.lib
    ${MAKE} obj\bci_filtertool.obj
	${LD} $< obj\bci_filtertool.obj obj\bci_tool.lib
	
.cpp.obj:
	${CPP} -c -o${PATHOBJ}\${*F}.obj $<

all: ${TARGETS}

obj\bci_tool.lib: ${BCIOBJ}
	&${AR} $@ +$**

clean:
	-${RM} ${PATHOBJ}\*.obj ${PATHOBJ}\*.lib ${PATHOBJ}\*.MAK

distclean: clean
    -${RM} *.exe *.~* *.tds *.il*
