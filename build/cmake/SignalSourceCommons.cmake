###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Defines the available signal source components for inclusion
##              in a signal source application

SET( BCI2000_SIGSRCSOURCES
  ${BCI2000_SIGSRCSOURCES}
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/AlignmentFilter.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/DataIOFilter.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/TransmissionFilter.cpp
)

SET( BCI2000_SIGSRCHEADERS
  ${BCI2000_SIGSRCHEADERS}
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/AlignmentFilter.h
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/DataIOFilter.h
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/TransmissionFilter.h
)

SET( BCI2000_SIGSRCSOURCES_EXTENSIONS
  ${BCI2000_SIGSRCSOURCES_EXTENSIONS}
)

SET( BCI2000_SIGSRCHEADERS_EXTENSIONS
  ${BCI2000_SIGSRCHEADERS_EXTENSIONS}
)


SET( BCI2000_SIGSRCINCDIRS
  ${BCI2000_SIGSRCINCDIRS}
)

SET( BCI2000_SIGSRCLIBDIRS 
  ${BCI2000_SIGSRCLIBDIRS}
)

SET( BCI2000_SIGSRCLIBS
  ${BCI2000_SIGSRCLIBS}
)

SET( BCI2000_SIGSRCDLLS
  ${BCI2000_SIGSRCDLLS}
)

SET( BCI2000_SIGSRCSOURCES_LOGGING
  ${BCI2000_SIGSRCSOURCES_LOGGING}
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/EventLink.cpp
)

SET( BCI2000_SIGSRCHEADERS_LOGGING
  ${BCI2000_SIGSRCHEADERS_LOGGING}
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/EventLink.h
)

IF( WIN32 )
  SET( BCI2000_SIGSRCSOURCES_LOGGING
    ${BCI2000_SIGSRCSOURCES_LOGGING}
    ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/JoystickLogger.cpp
    ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/KeyLogger.cpp
  )
  SET( BCI2000_SIGSRCHEADERS_LOGGING
    ${BCI2000_SIGSRCHEADERS_LOGGING}
    ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/JoystickLogger.h
    ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/KeyLogger.h
  )
ENDIF( WIN32 )
