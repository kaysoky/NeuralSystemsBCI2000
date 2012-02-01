####################################################################
# $Id$
# Author:      juergen.mellinger@uni-tuebingen.de
# Description: Makefile for BCI2000 Matlab mex files.
#
# $BEGIN_BCI2000_LICENSE$
# 
# This file is part of BCI2000, a platform for real-time bio-signal research.
# [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
# 
# BCI2000 is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
# 
# BCI2000 is distributed in the hope that it will be useful, but
#                         WITHOUT ANY WARRANTY
# - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.
# 
# $END_BCI2000_LICENSE$
####################################################################

AND = &&
OR  = ||

TARGETS = obj\bci_mex.lib \
          load_bcidat.mexw32 \
          save_bcidat.mexw32 \
          convert_bciprm.mexw32 \
          mem.mexw32 \
                    
MEXOBJ  = obj\bci_stubs.obj \
          obj\BCI2000FileReader.obj \
          obj\BCI2000OutputFormat.obj \
          obj\EDFOutputFormat.obj \
          obj\GDFOutputFormat.obj \
          obj\EDFOutputBase.obj \
          obj\GDF.obj \
          obj\Param.obj \
          obj\ParamList.obj \
          obj\ParamRef.obj \
          obj\State.obj \
          obj\StateList.obj \
          obj\StateVector.obj \
          obj\StateVectorSample.obj \
          obj\GenericSignal.obj \
          obj\SignalProperties.obj \
          obj\SignalType.obj \
          obj\PhysicalUnit.obj \
          obj\MeasurementUnits.obj \
          obj\ArithmeticExpression.obj \
          obj\Expression.obj \
          obj\ExpressionParser.obj \
          obj\EncodedString.obj \
          obj\LabelIndex.obj \
          obj\HierarchicalLabel.obj \
          obj\Brackets.obj \
          obj\MessageHandler.obj \
          obj\SysCommand.obj \
          obj\ClassName.obj \
          obj\VersionInfo.obj \
          obj\Environment.obj \
          obj\GenericFilter.obj \
          obj\BCIError.obj \
          obj\BCIError_mex.obj \
          obj\ARGroup.obj \
          obj\ARChannel.obj \
          obj\mexutils.obj \
          
MATLIB = libmx.lib libmex.lib libmat.lib
          
BCIROOT = ..\..\..
CMDLINE = ${BCIROOT}\core\Tools\cmdline
SRCROOT = ${BCIROOT}\shared\modules\signalsource
SIGROOT = ${BCIROOT}\shared\modules\signalprocessing
SIGPROC = ${BCIROOT}\shared\modules;${SIGROOT};${BCIROOT}\extlib\fftlib
PATHOBJ = .\obj
PATHCPP = .;${BCIROOT}\shared;${BCIROOT}\shared\accessors;${BCIROOT}\shared\fileio\dat;${BCIROOT}\shared\fileio\edf_gdf;${BCIROOT}\shared\types;${BCIROOT}\shared\bcistream;${BCIROOT}\shared\utils;${BCIROOT}\shared\utils\Expression;${SIGPROC};${SRCROOT};${CMDLINE};${BCIROOT}\extlib\math
INCLUDES = -I${BCIROOT};${BCIROOT}\shared\modules;${BCIROOT}\shared;${BCIROOT}\extlib\math; \
           -I${BCIROOT}\shared\accessors;${BCIROOT}\shared\bcistream;${BCIROOT}\shared\config; \
           -I${BCIROOT}\shared\fileio;${BCIROOT}\shared\fileio\dat;${BCIROOT}\shared\fileio\edf_gdf;${BCIROOT}\shared\types;${BCIROOT}\shared\utils;${BCIROOT}\shared\utils\Expression; \
           -I${SIGROOT} \
           -I${BCB}\include;${BCB}\include\vcl;${MATLABPATH}
LIBPATH = ${BCB}\lib\obj;${BCB}\lib;${MATLABPATH}
MATLABPATH = ${BCIROOT}\extlib\matlab 

BINDIR = ..\..\..\..\Tools\mex

SUPPL = load_bcidat.m convert_bciprm.m

DEFINES = -D_DEBUG;MATLAB_MEX_FILE;BCI_TOOL;BCI_MEX;NO_STRICT;_NO_VCL
CFLAGS = -Od -Vx -Ve -X- -r- -a8 -b- -k -y -v -vi- -c -3 -P- -w- -pc -tW -tWM -w-par

CPP = bcc32 ${CFLAGS} ${DEFINES} ${INCLUDES}
LD  = bcc32 -laa -lc -lTpd -lx -lGn -L${LIBPATH}
AR  = tlib /C /P256 /0 >nul # If this fails, try a larger value for the /P argument.
RM  = del 

.autodepend
.path.cpp=${PATHCPP}
	
{$(PATHOBJ)}.obj.mexw32:
	${MAKE} obj\bci_mex.lib
	${LD} -WD "$<" obj\bci_mex.lib ${MATLIB}
	${SHELL} move "$&.dll" "$&.mexw32"

.cpp.obj:
	if not exist ${PATHOBJ} mkdir ${PATHOBJ}
	${CPP} -c -o${PATHOBJ}\${*F}.obj $<

.mexw32.install:
  ${SHELL} if not exist ${BINDIR} mkdir ${BINDIR}
  ${SHELL} move "$<" ${BINDIR}
  ${DONE}

all: ${TARGETS} 

install: all ${TARGETS:.mexw32=.install}
	for %i in (${SUPPL}) do copy /y "%i" "${BINDIR}"

obj\bci_mex.lib: ${MEXOBJ}
	&${AR} $@ +$**

clean:
	-${RM} ${PATHOBJ}\*.obj ${PATHOBJ}\*.lib ${PATHOBJ}\*.BAK *.tds

distclean: clean
	-${RM} *.exe *.mexw32 *.~* *.il*
