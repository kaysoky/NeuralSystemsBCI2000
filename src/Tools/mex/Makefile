####################################################################
# $Id$
# File:        Makefile
# Date:        Jan 17, 2006
# Author:      juergen.mellinger@uni-tuebingen.de
# Description: Makefile for BCI2000 Matlab mex files.
# $Log$
# Revision 1.2  2006/08/10 15:36:23  mellinger
# Extended parameter translation into Matlab; introduced partial file reading.
#
# Revision 1.1  2006/01/17 17:15:47  mellinger
# Initial version.
#
# (C) 2000-2007, BCI2000 Project
# http://www.bci2000.org
####################################################################

AND = &&
OR  = ||

TARGETS = obj\bci_mex.lib \
          load_bcidat.dll \
          convert_bciprm.dll \
                    
MEXOBJ  = obj\bci_stubs.obj \
          obj\UBCI2000Data.obj \
          obj\UParameter.obj \
          obj\EncodedString.obj \
          obj\UState.obj \
          obj\UGenericSignal.obj \
          obj\UBCIError_mex.obj \
          obj\mexutils.obj \
          
MATLIB = libmx.lib libmex.lib libmat.lib
          
BCIROOT = ..\..
CMDLINE = ${BCIROOT}\Tools\cmdline
SRCROOT = ${BCIROOT}\EEGSource
SIGROOT = ${BCIROOT}\SignalProcessing
SIGPROC = ${SIGROOT};${SIGROOT}\AR;${SIGROOT}\FIR;${SIGROOT}\P3;${SIGROOT}\PeakDetector;${SIGROOT}\SW;${SIGROOT}\fftlib;${SIGROOT}\misc
PATHOBJ = .\obj
PATHCPP = .;${BCIROOT}\shared;${BCIROOT}\shared\Expression;${SIGPROC};${SRCROOT};${CMDLINE}
INCLUDEPATH = ${BCIROOT};${BCIROOT}\shared;${BCB}\include;${BCB}\include\vcl;${MATLABPATH}
LIBPATH = ${BCB}\lib\obj;${BCB}\lib;${MATLABPATH}
MATLABPATH = ${BCIROOT}\lib\matlab 

BINDIR = ..\..\..\Tools\mex

SUPPL = load_bcidat.m convert_bciprm.m

DEFINES = _DEBUG;MATLAB_MEX_FILE;BCI_TOOL;BCI_MEX;NO_STRICT;_NO_VCL
CFLAGS = -Od -Vx -Ve -X- -r- -a8 -b- -k -y -v -vi- -c -tW -tWM -w-par

CPP = bcc32 ${CFLAGS} -D${DEFINES} -I${INCLUDEPATH}
LD  = bcc32 -L${LIBPATH}
AR  = tlib /C /P128 /0 >nul # If this fails, try a larger value for the /P argument.
RM  = del 

.autodepend
.path.cpp=${PATHCPP}
	
{$(PATHOBJ)}.obj.dll:
	${MAKE} obj\bci_mex.lib
	${LD} -WD $< obj\bci_mex.lib ${MATLIB}

.cpp.obj:
	if not exist ${PATHOBJ} mkdir ${PATHOBJ}
	${CPP} -c -o${PATHOBJ}\${*F}.obj $<

.dll.install:
  ${SHELL} if not exist ${BINDIR} mkdir ${BINDIR}
  ${SHELL} move "$<" ${BINDIR}
  ${DONE}

all: ${TARGETS} 

install: all ${TARGETS:.dll=.install}
	for %i in (${SUPPL}) do copy /y "%i" "${BINDIR}"

obj\UBCIError_mex.obj: ${BCIROOT}\shared\UBCIError.cpp
	if not exist ${PATHOBJ} mkdir ${PATHOBJ}
	${CPP} -c -o${PATHOBJ}\${*F}.obj ${BCIROOT}\shared\UBCIError.cpp

obj\bci_mex.lib: ${MEXOBJ}
	&${AR} $@ +$**

clean:
	-${RM} ${PATHOBJ}\*.obj ${PATHOBJ}\*.lib ${PATHOBJ}\*.BAK *.tds

distclean: clean
	-${RM} *.exe *.~* *.il*
