###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including the pthreads-win32 API
## SETS:
##       SRC_EXTLIB_PTHREADS - Required source files for pthreads
##       HDR_EXTLIB_PTHREADS - Required header files for pthreads
##       INC_EXTLIB_PTHREADS - Include directory for pthreads
##       LIBDIR_EXTLIB_PTHREADS - Library directory for pthreads
##       LIBS_EXTLIB_PTHREADS - required library for pthreads
##       Also defines source groups for source files

IF( WIN32 )

# Set the Source and headers
SET( SRC_EXTLIB_PTHREADS )
SET( HDR_EXTLIB_PTHREADS
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include/pthread.h
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include/sched.h
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include/semaphore.h
)

# Define the include directory
SET( INC_EXTLIB_PTHREADS 
  ${BCI2000_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include
)

# Define where the library is
SET( LIBDIR_EXTLIB_PTHREADS ${BCI2000_SRC_DIR}/extlib/fieldtrip/pthreads-win32/lib )

# Set Libs required
IF( MINGW )
  SET( LIBS_EXTLIB_PTHREADS libpthreadGC2.a )
ELSEIF( MSVC )
  SET( LIBS_EXTLIB_PTHREADS pthreadVC2.lib )
ELSEIF( BORLAND )
  SET( LIBS_EXTLIB_PTHREADS pthreadVC2.bcb.lib )
ENDIF()

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\fieldtrip\\pthreads-win32 FILES ${SRC_EXTLIB_PTHREADS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\fieldtrip\\pthreads-win32 FILES ${HDR_EXTLIB_PTHREADS} )

# Set success
SET( PTHREADS_OK TRUE )

ENDIF( WIN32 )
