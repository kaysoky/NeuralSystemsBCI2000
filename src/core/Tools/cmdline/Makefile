####################################################################
# $Id$
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
#
# (C) 2000-2008, BCI2000 Project
# http://www.bci2000.org
####################################################################

!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

AND = &&
OR  = ||

TARGETS = obj\bci_tool.lib \
          obj\bci_dll.lib \
          bci_dat2stream.exe \
          bci_prm2stream.exe \
          bci_stream2prm.exe \
          bci_stream2asc.exe \
          bci_stream2table.exe \
          bci_stream2mat.exe \
          SpatialFilter.exe \
          ARFilter.exe \
          P3TemporalFilter.exe \
          FFTFilter.exe \
          TransmissionFilter.exe \
          LinearClassifier.exe \
          Normalizer.exe \
          MatlabFilter.exe \
          ConditionalIntegrator.exe \
          ComplexDemodulator.exe \
          StateTransform.exe \
          SpatialFilter.dll \
          ARFilter.dll \
          P3TemporalFilter.dll \
          FFTFilter.dll \
          TransmissionFilter.dll \
          LinearClassifier.dll \
          Normalizer.dll \
          MatlabFilter.dll \
          ConditionalIntegrator.dll \
          ComplexDemodulator.dll \
          StateTransform.dll \
          
BCIOBJ  = obj\bci_stubs.obj \
          obj\Param.obj \
          obj\ParamList.obj \
          obj\State.obj \
          obj\StateList.obj \
          obj\StateVector.obj \
          obj\EncodedString.obj \
          obj\ParamRef.obj \
          obj\GenericSignal.obj \
          obj\SignalProperties.obj \
          obj\SignalType.obj \
          obj\Environment.obj \
          obj\GenericFilter.obj \
          obj\GenericVisualization.obj \
          obj\Label.obj \
          obj\LabelIndex.obj \
          obj\HierarchicalLabel.obj \
          obj\Brackets.obj \
          obj\PhysicalUnit.obj \
          obj\Color.obj \
          obj\BCIError.obj \
          obj\BCIError_tool.obj \
          obj\State.obj \
          obj\Status.obj \
          obj\SysCommand.obj \
          obj\MessageHandler.obj \
          obj\MeasurementUnits.obj \
          obj\Expression.obj \
          obj\ExpressionParser.obj \
          obj\ClassName.obj \
          obj\FFTLibWrap.obj \
          obj\MatlabWrapper.obj \
          obj\RandomGenerator.obj \
          
          
BCIROOT = ..\..\..
SRCROOT = ${BCIROOT}\shared\modules\signalsource
SIGROOT = ${BCIROOT}\shared\modules\signalprocessing
BCILIB = "${BCIROOT}";"${BCIROOT}\shared";"${BCIROOT}\shared\config";"${BCIROOT}\shared\accessors";"${BCIROOT}\shared\bcistream";"${BCIROOT}\shared\fileio";"${BCIROOT}\shared\types";"${BCIROOT}\shared\modules";"${BCIROOT}\shared\utils";"${BCIROOT}\shared\utils\Expression"
EXTLIB = ${BCIROOT}\extlib
SIGPROC = "${SIGROOT}";"${BCIROOT}\core\SignalProcessing\Matlab";"${EXTLIB}\fftlib"
PATHOBJ = .\obj
PATHCPP = .;"${BCILIB}";"${SIGPROC}";"${SRCROOT}";"${EXTLIB}"
INCLUDEPATH = "${SRCROOT}";"${BCILIB}";"${EXTLIB}\matlab";"${EXTLIB}\math";"${EXTLIB}\fftlib";"${BCB}\include";"${BCB}\include\vcl"
LIBPATH = "${BCB}\lib\obj";"${BCB}\lib"

BINDIR = ..\..\..\..\Tools\cmdline
DLLDIR = ..\..\..\..\Tools\dll

SCRIPTS = *.cmd *.bat load_bcimat.m
DLLSUPPL = dll_example.m bci_dll.h

DEFINES = _DEBUG;BCI_TOOL;NO_STRICT;_NO_VCL;SIGNAL_BACK_COMPAT
CFLAGS = -Od -Vx -Ve -X- -r- -a8 -b- -k -y -v -vi- -c -tW -tWM -w-par

CPP = bcc32 ${CFLAGS} -D${DEFINES} -I${INCLUDEPATH}
LD  = bcc32 -L${LIBPATH}
AR  = tlib /C /P256 /0 >nul # If this fails, try a larger value for the /P argument.
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

obj\bci_tool.lib: obj\bci_tool.obj ${BCIOBJ}
	&${AR} $@ +$**

obj\bci_dll.lib: obj\bci_dll.obj ${BCIOBJ}
	&${AR} $@ +$**

clean:
	-${RM} ${PATHOBJ}\*.obj ${PATHOBJ}\*.lib ${PATHOBJ}\*.BAK *.tds

distclean: clean
	-${RM} *.exe *.dll *.~* *.il*
