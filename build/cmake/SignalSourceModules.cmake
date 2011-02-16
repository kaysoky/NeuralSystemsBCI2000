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
ENDIF( WIN32 )
