###########################################################################
## $Id$
## Authors: jezhill@gmail.com

IF( NOT WIN32 )
	MESSAGE( "**** DataGloveLogger failed: fglove.dll not found for this platform" )
	RETURN()
ENDIF()

SET( BCI2000_SIGSRCLIBS
   ${BCI2000_SIGSRCLIBS}
)

SET( BCI2000_SIGSRCLIBDIRS
   ${BCI2000_SIGSRCLIBDIRS}
)

SET( BCI2000_SIGSRCINCDIRS
   ${BCI2000_SIGSRCINCDIRS}
   ${BCI2000_EXTENSION_DIR}
   ${BCI2000_EXTENSION_DIR}/extlib/include
)

SET( BCI2000_SIGSRCHEADERS_EXTENSIONS
   ${BCI2000_SIGSRCHEADERS_EXTENSIONS}
   ${BCI2000_EXTENSION_DIR}/5DTDataGloveUltraLogger.h
)

SET( BCI2000_SIGSRCSOURCES_EXTENSIONS
   ${BCI2000_SIGSRCSOURCES_EXTENSIONS}
   ${BCI2000_EXTENSION_DIR}/5DTDataGloveUltraLogger.cpp
)

SET( BCI2000_SIGSRCDLLS
   ${BCI2000_SIGSRCDLLS}
   ${BCI2000_EXTENSION_DIR}/extlib/dylib/fglove.dll
)
