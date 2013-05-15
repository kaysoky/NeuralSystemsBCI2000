###########################################################################
## $Id: IncludeExtension.cmake 3348 2011-06-24 15:41:55Z mellinger $
## Authors: griffin.milsap@gmail.com

SET( BCI2000_SIGSRCLIBS
  ${BCI2000_SIGSRCLIBS}
  PortAudio
  Sndfile
)

SET( BCI2000_SIGSRCINCDIRS
  ${BCI2000_SIGSRCINCDIRS}
  ${BCI2000_SRC_DIR}/extlib/math
  ${BCI2000_SRC_DIR}/extlib/libsndfile/include
  ${BCI2000_SRC_DIR}/extlib/portaudio/portaudio/include
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
