###############################################################################
# $Id$
# File: Makefile
# Author: juergen.mellinger@uni-tuebingen.de
# Description: Makefile for the split_bcidat tool.
#
# (C) 2000-2010, BCI2000 Project
# http://www.bci2000.org
###############################################################################
AND = &&
OR  = ||

TARGETS = split_bcidat.exe
          
OBJ = obj\split_bcidat.obj 
          
BCIROOT = ..\..\..
PATHOBJ = .\obj
PATHCPP = .
INCLUDEPATH = ${PATHCPP};${BCB}\include;${BCB}\include\vcl
LIBPATH = ${BCB}\lib\obj;${BCB}\lib

DEFINES = _DEBUG;NO_STRICT;_NO_VCL
CFLAGS = -Od -Vx -Ve -X- -r- -a8 -b- -k -y -v -vi- -c -tW -tWM -w-par

CPP = bcc32 ${CFLAGS} -D${DEFINES} -I${INCLUDEPATH}
LD  = bcc32 -L${LIBPATH}
RM  = del 

.autodepend
.path.cpp=${PATHCPP}

split_bcidat.exe: ${OBJ}
	${LD} ${OBJ}

.cpp.obj:
	if not exist ${PATHOBJ} mkdir ${PATHOBJ}
	${CPP} -c -o${PATHOBJ}\${*F}.obj $<

all: ${TARGETS}

clean:
	-${RM} ${PATHOBJ}\*.obj ${PATHOBJ}\*.lib ${PATHOBJ}\*.BAK *.tds

distclean: clean
	-${RM} *.exe *.~* *.il*
