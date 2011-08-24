###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including the FieldTrip library
## SETS:
##       SRC_EXTLIB - Required source files for FieldTrip
##       HDR_EXTLIB - Required header files for FieldTrip
##       INC_EXTLIB - Include directory for FieldTrip
##       Also defines source groups for source files

# Set the Source and headers
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/cleanup.c
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/clientrequest.c
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/dmarequest.c
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/endianutil.c
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/extern.c
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/printstruct.c
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/swapbytes.c
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/tcprequest.c
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/tcpserver.c
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/tcpsocket.c
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/util.c
)
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/buffer.h
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/compiler.h
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/extern.h
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/message.h
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/platform.h
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/platform_includes.h
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/swapbytes.h
)

IF( WIN32 )
SET( SRC_EXTLIB
  ${SRC_EXTLIB}
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/win32/gettimeofday.c
)
SET( HDR_EXTLIB
  ${HDR_EXTLIB}
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/win32/gettimeofday.h
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/win32/stdint.h
)
ENDIF( WIN32 )

# Define the include directory
SET( INC_EXTLIB 
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/buffer/src/
)

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\fieldtrip\\buffer FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\fieldtrip\\buffer FILES ${HDR_EXTLIB} )

# Set success
SET( EXTLIB_OK TRUE )
