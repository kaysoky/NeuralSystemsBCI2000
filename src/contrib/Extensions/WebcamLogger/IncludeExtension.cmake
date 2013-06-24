###########################################################################
## $Id: IncludeExtension.cmake 3069 2011-01-19 23:27:37Z jhill $
## Authors: jezhill@gmail.com

IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
  IF( MSVC )
    SET( OPENCV_LIBDIR ${BCI2000_SRC_DIR}/extlib/opencv/lib/msvc )
  ELSEIF( MINGW )
    SET( OPENCV_LIBDIR ${BCI2000_SRC_DIR}/extlib/opencv/lib/mingw )
   ENDIF()
ENDIF()
IF( NOT OPENCV_LIBDIR )
  MESSAGE( "**** WebcamLogger failed: opencv libraries not found for this platform" )
  RETURN()
ENDIF()

#ADD_DEFINITIONS( -DUSE_QT )

SET( BCI2000_SIGSRCLIBS
   ${BCI2000_SIGSRCLIBS}
   opencv_core220
   opencv_highgui220
   opencv_ffmpeg220
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

SET( BCI2000_SIGSRCHEADERS_EXTENSIONS
   ${BCI2000_SIGSRCHEADERS_EXTENSIONS}
   ${BCI2000_EXTENSION_DIR}/WebcamLogger.h
)

SET( BCI2000_SIGSRCSOURCES_EXTENSIONS
   ${BCI2000_SIGSRCSOURCES_EXTENSIONS}
   ${BCI2000_EXTENSION_DIR}/WebcamLogger.cpp
)

IF( MSVC )
SET( BCI2000_SIGSRCDLLS
   ${BCI2000_SIGSRCDLLS}
   ${OPENCV_LIBDIR}/opencv_core220.dll
   ${OPENCV_LIBDIR}/opencv_highgui220.dll
   ${OPENCV_LIBDIR}/opencv_ffmpeg220.dll
)
ELSEIF(MINGW)
SET( BCI2000_SIGSRCDLLS
   ${BCI2000_SIGSRCDLLS}
   ${OPENCV_LIBDIR}/libopencv_core220.dll
   ${OPENCV_LIBDIR}/libopencv_highgui220.dll
   ${OPENCV_LIBDIR}/libopencv_ffmpeg220.dll
)
ENDIF(MSVC)
