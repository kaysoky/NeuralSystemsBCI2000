###########################################################################
## $Id$
## Authors: Adam Wilson
## Description: Build information for BCI2000Certification

IF( BUILD_TESTS )

# Set the executable name
SET( NAME BCI2000Certification)

MESSAGE( "-- Adding Project: ${NAME}" )

# Set the project specific sources
SET( SOURCES
  certificationGUI.cpp
  certificationMain.cpp
  certLauncher.cpp
  ../TaskType.cpp
  ../Functions.cpp
  ../progressClass.cpp
)


INCLUDE_DIRECTORIES( . ../ )
SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/tools/BCI2000Certification )

BCI2000_ADD_TARGET( QTAPP ${NAME} ${SOURCES} )
# Set the project build folder
SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER Tests/Certification )

ENDIF( BUILD_TESTS )
