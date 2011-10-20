###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including the pthreads-win32 API
## SETS:
##       SRC_EXTLIB - Required source files for pthreads
##       HDR_EXTLIB - Required header files for pthreads
##       INC_EXTLIB - Include directory for pthreads
##       LIBDIR_EXTLIB - Library directory for pthreads
##       LIBS_EXTLIB - required library for pthreads
##       Also defines source groups for source files

IF( WIN32 AND NOT CMAKE_CL_64 )

# Set the Source and headers
SET( SRC_EXTLIB )
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include/pthread.h
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include/sched.h
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include/semaphore.h
)

# Define the include directory
SET( INC_EXTLIB 
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include
)

# Define where the library is
SET( LIBDIR_EXTLIB ${BCI2000_SRC_DIR}/extlib/fieldtrip/pthreads-win32/lib )

# Set Libs required
IF( MINGW )
  SET( LIBS_EXTLIB libpthreadGC2.a )
ELSEIF( MSVC )
  SET( LIBS_EXTLIB pthreadVC2.lib )
ELSEIF( BORLAND )
  SET( LIBS_EXTLIB pthreadVC2.bcb.lib )
ENDIF()

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\fieldtrip\\pthreads-win32 FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\fieldtrip\\pthreads-win32 FILES ${HDR_EXTLIB} )

# Set success
SET( EXTLIB_OK TRUE )

ENDIF()
