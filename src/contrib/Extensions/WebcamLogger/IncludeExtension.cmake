###########################################################################
## $Id: IncludeExtension.cmake 3069 2011-01-19 23:27:37Z jhill $
## Authors: jezhill@gmail.com

IF( MSVC )
	SET( OPENCV_LIBDIR
		${BCI2000_SRC_DIR}/extlib/opencv/lib/msvc
	)
#ELSEIF( MINGW )
#	SET( OPENCV_LIBDIR
#		${BCI2000_SRC_DIR}/extlib/opencv/lib/mingw
#	)
ELSE()
	MESSAGE( "**** WebcamLogger failed: opencv libraries not found for this platform" )
	RETURN()
ENDIF()

SET( BCI2000_SIGSRCLIBS
   ${BCI2000_SIGSRCLIBS}
   opencv_core220
   opencv_highgui220
)

SET( BCI2000_SIGSRCLIBDIRS
   ${BCI2000_SIGSRCLIBDIRS}
   ${OPENCV_LIBDIR}
)

SET( BCI2000_SIGSRCINCDIRS
   ${BCI2000_SIGSRCINCDIRS}
   ${BCI2000_EXTENSION_DIR}
   ${BCI2000_SRC_DIR}/extlib/opencv/modules/core/include
   ${BCI2000_SRC_DIR}/extlib/opencv/modules/core/include/opencv2/core
   ${BCI2000_SRC_DIR}/extlib/opencv/modules/highgui/include
   ${BCI2000_SRC_DIR}/extlib/opencv/modules/highgui/include/opencv2/highgui
)

SET( BCI2000_SIGSRCHEADERS
   ${BCI2000_SIGSRCHEADERS}
   ${BCI2000_EXTENSION_DIR}/WebcamLogger.h
)

SET( BCI2000_SIGSRCSOURCES
   ${BCI2000_SIGSRCSOURCES}
   ${BCI2000_EXTENSION_DIR}/WebcamLogger.cpp
)
