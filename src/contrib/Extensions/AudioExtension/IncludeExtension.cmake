###########################################################################
## $Id: IncludeExtension.cmake 3348 2011-06-24 15:41:55Z mellinger $
## Authors: griffin.milsap@gmail.com

SET( BCI2000_SIGSRCLIBS
  ${BCI2000_SIGSRCLIBS}
  portaudio_x86
  libsndfile-1
)

SET( BCI2000_SIGSRCLIBDIRS
  ${BCI2000_SIGSRCLIBDIRS}
  ${BCI2000_EXTENSION_DIR}/portaudio/lib/coff
  ${BCI2000_EXTENSION_DIR}/libsndfile/lib/coff 
)

SET( BCI2000_SIGSRCINCDIRS
  ${BCI2000_SIGSRCINCDIRS}
  ${BCI2000_EXTENSION_DIR}
  ${BCI2000_EXTENSION_DIR}/portaudio/inc
  ${BCI2000_EXTENSION_DIR}/libsndfile/inc
  ${BCI2000_SRC_DIR}/extlib/math
)

SET( BCI2000_SIGSRCHEADERS_EXTENSIONS
  ${BCI2000_SIGSRCHEADERS_EXTENSIONS}
  ${BCI2000_EXTENSION_DIR}/AudioExtension.h
)

SET( BCI2000_SIGSRCSOURCES_EXTENSIONS
  ${BCI2000_SIGSRCSOURCES_EXTENSIONS}
  ${BCI2000_EXTENSION_DIR}/AudioExtension.cpp
  ${BCI2000_SRC_DIR}/extlib/math/FilterDesign.cpp
)

SET( BCI2000_SIGSRCDLLS
  ${BCI2000_SIGSRCDLLS}
  ${BCI2000_EXTENSION_DIR}/portaudio/dylib/portaudio_x86.dll
  ${BCI2000_EXTENSION_DIR}/libsndfile/dylib/libsndfile-1.dll
)
