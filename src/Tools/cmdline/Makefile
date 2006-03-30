####################################################################
# $Id$
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
# $Log$
# Revision 1.24  2006/03/30 14:50:38  mellinger
# Added ClassInfo object file.
#
# Revision 1.23  2006/03/30 10:22:33  mellinger
# Introduced quotes into paths.
#
# Revision 1.22  2006/02/22 19:26:41  mellinger
# Added Matlab filter and fixed BCB macro.
#
# Revision 1.21  2006/01/17 17:16:43  mellinger
# Added StateTransform filter.
#
####################################################################

!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

AND = &&
OR  = ||

TARGETS = obj\bci_tool.lib \
          bci_dat2stream.exe \
          bci_prm2stream.exe \
          bci_stream2prm.exe \
          bci_stream2asc.exe \
          bci_stream2table.exe \
          bci_stream2mat.exe \
#         bci_stream2dat.exe \
#         bci_stream2gdf.exe \
#         bci_stream2edf.exe \
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
          Normalizer.exe \
          MatlabFilter.exe \
          ConditionalIntegrator.exe \
          ComplexDemodulator.exe \
          StateTransform.exe \
          StatFilter.exe \
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
          Normalizer.dll \
          MatlabFilter.dll \
          ConditionalIntegrator.dll \
          ComplexDemodulator.dll \
          StateTransform.dll \
          StatFilter.dll \
          
BCIOBJ  = obj\bci_tool.obj \
          obj\bci_stubs.obj \
          obj\UParameter.obj \
          obj\EncodedString.obj \
          obj\ParamRef.obj \
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
          obj\Expression.obj \
          obj\ExpressionParser.obj \
          obj\ClassName.obj \
          obj\getmem.obj \
          obj\getfir.obj \
          obj\FFTLibWrap.obj \
          obj\MatlabWrapper.obj
          
DLLOBJ  = obj\bci_dll.obj \
          obj\bci_stubs.obj \
          obj\UParameter.obj \
          obj\ParamRef.obj \
          obj\EncodedString.obj \
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
          obj\Expression.obj \
          obj\ExpressionParser.obj \
          obj\ClassName.obj \
          obj\getmem.obj \
          obj\getfir.obj \
          obj\FFTLibWrap.obj \
          obj\MatlabWrapper.obj
          
STATOBJ = obj\StatFilter.obj \
          obj\Statistics.obj \
          obj\Normalfilter.obj \
          obj\ClassFilter.obj \
          
BCIROOT = ..\..
SRCROOT = ${BCIROOT}\EEGSource
SIGROOT = ${BCIROOT}\SignalProcessing
SIGPROC = "${SIGROOT}";"${SIGROOT}\AR";"${SIGROOT}\FIR";"${SIGROOT}\P3";"${SIGROOT}\PeakDetector";"${SIGROOT}\SW";"${SIGROOT}\fftlib";"${SIGROOT}\Matlab";"${SIGROOT}\misc"
PATHOBJ = .\obj
PATHCPP = .;"${BCIROOT}\shared";"${BCIROOT}\shared\Expression";${SIGPROC};"${SRCROOT}"
INCLUDEPATH = "${BCIROOT}";"${BCIROOT}\shared";"${SRCROOT}";"${BCIROOT}\lib\matlab";"${BCB}\include";"${BCB}\include\vcl"
LIBPATH = "${BCB}\lib\obj";"${BCB}\lib"

BINDIR = ..\..\..\Tools\cmdline
DLLDIR = ..\..\..\Tools\dll

SCRIPTS = *.cmd *.bat load_bcimat.m
DLLSUPPL = dll_example.m bci_dll.h

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
	for %i in (${DLLSUPPL}) do copy /y "%i" "${DLLDIR}"

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

StatFilter.exe: obj\StatFilter.lib obj\bci_tool.lib obj\bci_filtertool.obj
	${LD} obj\StatFilter.obj obj\StatFilter.lib obj\bci_tool.lib obj\bci_filtertool.obj rtl.lib
	
StatFilter.dll: obj\StatFilter.lib obj\bci_tool.lib obj\bci_dll.lib
	${LD} -WD obj\StatFilter.obj obj\StatFilter.lib obj\bci_dll.lib rtl.lib
	
clean:
	-${RM} ${PATHOBJ}\*.obj ${PATHOBJ}\*.lib ${PATHOBJ}\*.BAK *.tds

distclean: clean
	-${RM} *.exe *.dll *.~* *.il*
