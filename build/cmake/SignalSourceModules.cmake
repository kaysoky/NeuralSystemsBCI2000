###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Defines the available signal source modules for inclusion
##              in a signal source application
##              Defines BCI2000_SIGSRCSOURCES/BCI2000_SIGSRCHEADERS

SET( BCI2000_SIGSRCSOURCES
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/AlignmentFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/DataIOFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/TransmissionFilter.cpp
)

SET( BCI2000_SIGSRCHEADERS
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/AlignmentFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/DataIOFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/TransmissionFilter.h
)

SET( BCI2000_SIGSRCINCDIRS

)

SET( BCI2000_SIGSRCLIBDIRS 

)

SET( BCI2000_SIGSRCLIBS

)

IF( WIN32 )
	
  SET( BCI2000_SIGSRCSOURCES
    ${BCI2000_SIGSRCSOURCES}
    ${BCI2000_SRC_DIR}/shared/modules/signalsource/JoystickLogger.cpp
    ${BCI2000_SRC_DIR}/shared/modules/signalsource/KeyLogger.cpp
  )
  SET( BCI2000_SIGSRCHEADERS
    ${BCI2000_SIGSRCHEADERS}
    ${BCI2000_SRC_DIR}/shared/modules/signalsource/JoystickLogger.h
    ${BCI2000_SRC_DIR}/shared/modules/signalsource/KeyLogger.h
  )
	IF (MSVC)
		SET( BCI2000_SIGSRCLIBDIRS 
		  ${BCI2000_SRC_DIR}/extlib/opencv/lib/
		)

		SET( BCI2000_SIGSRCLIBS
		  ${BCI2000_SRC_DIR}/extlib/opencv/lib/msvc/opencv_core220.lib
		  ${BCI2000_SRC_DIR}/extlib/opencv/lib/msvc/opencv_highgui220.lib
		)
	ENDIF(MSVC)
	IF (NOT BORLAND)
		SET( BCI2000_SIGSRCSOURCES
			${BCI2000_SIGSRCSOURCES}
			${BCI2000_SRC_DIR}/shared/modules/signalsource/WebcamLogger.cpp
		)
		SET( BCI2000_SIGSRCHEADERS
			${BCI2000_SIGSRCHEADERS}
			${BCI2000_SRC_DIR}/shared/modules/signalsource/WebcamLogger.h
		)
		INCLUDE_DIRECTORIES(
			${BCI2000_SRC_DIR}/extlib/opencv/include
			${BCI2000_SRC_DIR}/extlib/opencv/modules/core/include
			${BCI2000_SRC_DIR}/extlib/opencv/modules/highgui/include
		)
	ENDIF (NOT BORLAND)
ENDIF( WIN32 )
