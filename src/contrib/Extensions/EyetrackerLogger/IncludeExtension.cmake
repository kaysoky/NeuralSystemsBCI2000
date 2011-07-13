###########################################################################
## $Id: IncludeExtension.cmake 3069 2011-01-19 23:27:37Z jhill $
## Authors: jezhill@gmail.com, griffin.milsap@gmail.com

IF( NOT WIN32 )
	MESSAGE( "**** EyetrackerLogger failed: tet.dll and ttime.dll not found for this platform" )
	RETURN()
ENDIF()

IF( MSVC OR MINGW )
	SET( TOBII_LIB_DIR  ${BCI2000_EXTENSION_DIR}/extlib/lib/msvc )
	SET( BCI2000_SIGSRCLIBS
	   ${BCI2000_SIGSRCLIBS}
	   "${TOBII_LIB_DIR}/cv.lib"
	   "${TOBII_LIB_DIR}/tet.lib"
	   "${TOBII_LIB_DIR}/ttime.lib"
	)
ELSEIF( BORLAND )
	SET( TOBII_LIB_DIR  ${BCI2000_EXTENSION_DIR}/extlib/lib/borland )
	SET( BCI2000_SIGSRCLIBS
	   ${BCI2000_SIGSRCLIBS}
	   "${TOBII_LIB_DIR}/cv_omf.lib"
	   "${TOBII_LIB_DIR}/tet_omf.lib"
	   "${TOBII_LIB_DIR}/ttime_omf.lib"
	)
ELSE( )
	MESSAGE( "**** EyetrackerLogger failed: tobii libraries (cv, tet, ttime) are not found for this platform/compiler" )
	RETURN()
ENDIF()

SET( BCI2000_SIGSRCLIBS
   ${BCI2000_SIGSRCLIBS}
)

SET( BCI2000_SIGSRCLIBDIRS
   ${BCI2000_SIGSRCLIBDIRS}
   ${TOBII_LIB_DIR}
)

SET( BCI2000_SIGSRCINCDIRS
   ${BCI2000_SIGSRCINCDIRS}
   ${BCI2000_EXTENSION_DIR}/extlib/include
   ${BCI2000_EXTENSION_DIR}
)

SET( BCI2000_SIGSRCHEADERS_EXTENSIONS
   ${BCI2000_SIGSRCHEADERS_EXTENSIONS}
   ${BCI2000_EXTENSION_DIR}/EyetrackerLogger.h
)

SET( BCI2000_SIGSRCSOURCES_EXTENSIONS
   ${BCI2000_SIGSRCSOURCES_EXTENSIONS}
   ${BCI2000_EXTENSION_DIR}/EyetrackerLogger.cpp
)

SET( BCI2000_SIGSRCDLLS
   ${BCI2000_SIGSRCDLLS}
   ${BCI2000_EXTENSION_DIR}/extlib/dylib/tet.dll
   ${BCI2000_EXTENSION_DIR}/extlib/dylib/ttime.dll
)

