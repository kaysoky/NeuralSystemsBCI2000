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
          bci_stream2prm.exe \
          bci_stream2asc.exe \
          bci_stream2table.exe \
          bci_stream2mat.exe \
          CalibrationFilter.exe \
          ClassFilter.exe \
          Normalfilter.exe \
          SpatialFilter.exe \
          ARFilter.exe \
          FIRFilter.exe \
          P3TemporalFilter.exe \
          UPeakDetector.exe \
          SWFilter.exe \
          FFTFilter.exe \
          TransmissionFilter.exe \
#         StatFilter.exe \
          CalibrationFilter.dll \
          ClassFilter.dll \
          Normalfilter.dll \
          SpatialFilter.dll \
          ARFilter.dll \
          FIRFilter.dll \
          P3TemporalFilter.dll \
          UPeakDetector.dll \
          SWFilter.dll \
          FFTFilter.dll \
          TransmissionFilter.dll \
#         StatFilter.dll \
          
BCIOBJ  = obj\bci_tool.obj \
          obj\bci_filterwrapper.obj \
          obj\bci_stubs.obj \
          obj\UParameter.obj \
          obj\UGenericSignal.obj \
          obj\UEnvironment.obj \
          obj\UGenericFilter.obj \
          obj\UGenericVisualization.obj \
          obj\UBCIError_tool.obj \
          obj\UState.obj \
          obj\UStatus.obj \
          obj\USysCommand.obj \
          obj\MessageHandler.obj \
          obj\MeasurementUnits.obj \
          obj\getmem.obj \
          obj\getfir.obj \
          obj\FFTLibWrap.obj 
          
DLLOBJ  = obj\bci_dll.obj \
          obj\bci_filterwrapper.obj \
          obj\bci_stubs.obj \
          obj\UParameter.obj \
          obj\UGenericSignal.obj \
          obj\UEnvironment.obj \
          obj\UGenericFilter.obj \
          obj\UGenericVisualization.obj \
          obj\UBCIError_dll.obj \
          obj\UState.obj \
          obj\UStatus.obj \
          obj\USysCommand.obj \
          obj\MessageHandler.obj \
          obj\MeasurementUnits.obj \
          obj\getmem.obj \
          obj\getfir.obj \
          obj\FFTLibWrap.obj 
          
STATOBJ = obj\bci_filtertool.obj \
          obj\StatFilter.obj \
          obj\Statistics.obj \
          obj\Normalfilter.obj \
          obj\ClassFilter.obj \

BCIROOT = ..\..
SRCROOT = ${BCIROOT}\EEGSource
SIGROOT = ${BCIROOT}\SignalProcessing
SIGPROC = ${SIGROOT};${SIGROOT}\AR;${SIGROOT}\FIR;${SIGROOT}\P3;${SIGROOT}\PeakDetector;${SIGROOT}\SW;${SIGROOT}\fftlib;${SIGROOT}\Old
PATHOBJ = .\obj
PATHCPP = .;${BCIROOT}\shared;${SIGPROC};${SRCROOT}
INCLUDEPATH = ${BCIROOT};${BCIROOT}\shared;${BCB}\include;${BCB}\include\vcl
LIBPATH = ${BCB}\lib\obj;${BCB}\lib

BINDIR = ..\..\..\Tools\cmdline
DLLDIR = ..\..\..\Tools\dll

SCRIPTS = *.cmd *.bat *.m

DEFINES = _DEBUG;BCI_TOOL;NO_STRICT;_NO_VCL;SIGNAL_BACK_COMPAT
CFLAGS = -Od -Vx -Ve -X- -r- -a8 -b- -k -y -v -vi- -c -tW -tWM -w-par

CPP = bcc32 ${CFLAGS} -D${DEFINES} -I${INCLUDEPATH}
LD  = bcc32 -L${LIBPATH}
AR  = tlib /C /P128 /0 >nul # If this fails, try a larger value for the /P argument.
RM  = del 

.autodepend
.path.cpp=${PATHCPP}

{$(PATHOBJ)}.obj.exe:
	${MAKE} obj\bci_tool.lib
	${MAKE} obj\bci_filtertool.obj
	${LD} $< obj\bci_filtertool.obj obj\bci_tool.lib
	
{$(PATHOBJ)}.obj.dll:
	${MAKE} obj\bci_dll.lib
	${LD} -WD $< obj\bci_dll.lib

.cpp.obj:
	if not exist ${PATHOBJ} mkdir ${PATHOBJ}
	${CPP} -c -o${PATHOBJ}\${*F}.obj $<

.exe.install:
  ${SHELL} if not exist ${BINDIR} mkdir ${BINDIR}
  ${SHELL} move "$<" ${BINDIR}
  ${DONE}

.dll.dllinstall:
  ${SHELL} if not exist ${DLLDIR} mkdir ${DLLDIR}
  ${SHELL} move "$<" ${DLLDIR}
  ${DONE}

all: ${TARGETS} 

install: all ${TARGETS:.exe=.install} ${TARGETS:.dll=.dllinstall}
	for %i in (${SCRIPTS}) do copy /y "%i" "${BINDIR}"

obj\bci_dll.obj: bci_dll.cpp
	if not exist ${PATHOBJ} mkdir ${PATHOBJ}
	${CPP} -DBCI_DLL -c -o${PATHOBJ}\${*F}.obj bci_dll.cpp   

obj\UBCIError_tool.obj: ${BCIROOT}\shared\UBCIError.cpp
	if not exist ${PATHOBJ} mkdir ${PATHOBJ}
	${CPP} -c -o${PATHOBJ}\${*F}.obj ${BCIROOT}\shared\UBCIError.cpp

obj\UBCIError_dll.obj: ${BCIROOT}\shared\UBCIError.cpp 
	if not exist ${PATHOBJ} mkdir ${PATHOBJ}
	${CPP} -DBCI_DLL -c -o${PATHOBJ}\${*F}.obj ${BCIROOT}\shared\UBCIError.cpp   

obj\bci_tool.lib: ${BCIOBJ}
	&${AR} $@ +$**

obj\bci_dll.lib: ${DLLOBJ}
	&${AR} $@ +$**

obj\StatFilter.lib: ${STATOBJ}
	&${AR} $@ +$**

StatFilter.exe: obj\StatFilter.lib obj\bci_tool.lib
	${LD} obj\StatFilter.obj obj\StatFilter.lib obj\bci_tool.lib rtl.lib
	
clean:
	-${RM} ${PATHOBJ}\*.obj ${PATHOBJ}\*.lib ${PATHOBJ}\*.BAK *.tds

distclean: clean
	-${RM} *.exe *.~* *.il*
