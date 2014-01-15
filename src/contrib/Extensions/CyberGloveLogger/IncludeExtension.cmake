###########################################################################
## $Id: IncludeExtension.cmake 3348 2011-06-24 15:41:55Z mellinger $
## Authors: tblakely

IF( NOT WIN32 )
	MESSAGE( "**** CyberGloveLogger failed: No platform support for non-Windows" )
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
   ${BCI2000_EXTENSION_DIR}/CyberGloveLogger.h
)

SET( BCI2000_SIGSRCSOURCES_EXTENSIONS
   ${BCI2000_SIGSRCSOURCES_EXTENSIONS}
   ${BCI2000_EXTENSION_DIR}/CyberGloveLogger.cpp
)