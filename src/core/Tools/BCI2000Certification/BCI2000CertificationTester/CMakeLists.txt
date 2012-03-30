###########################################################################
## $Id$
## Authors: Adam Wilson
## Description: Build information for BCI2000Certification

# Set the executable name
SET( EXECUTABLE_NAME BCI2000Certification)

# DEBUG
MESSAGE( "-- Adding Project: ${EXECUTABLE_NAME}" )

# Set the project specific sources
SET( SRC_GROUP
  certificationGUI.cpp
  certificationMain.cpp
  certLauncher.cpp
  ../TaskType.cpp
  ../Functions.cpp
  ../progressClass.cpp
)
SET( HDR_GROUP
  certificationGUI.h
  certLauncher.h
  ../TaskType.h
  ../Functions.h
  ../progressClass.h
)

# Wrap the Qt files
QT4_WRAP_CPP( GENERATED_MOC certificationGUI.h ../progressClass.h)
SET( GENERATED
  ${GENERATED_MOC}
)

INCLUDE_DIRECTORIES( . ../ )
SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/tools/BCI2000Certification )

# Create the signal processing module
ADD_EXECUTABLE( ${EXECUTABLE_NAME} WIN32 ${SRC_GROUP} ${HDR_GROUP} ${GENERATED} )

# Add Pre-processor defines
SET_PROPERTY( TARGET ${EXECUTABLE_NAME} APPEND PROPERTY COMPILE_FLAGS "-DUSE_QT" )

TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${QT_LIBRARIES} )

# Set the project build folder
SET_PROPERTY( TARGET ${EXECUTABLE_NAME} PROPERTY FOLDER "${DIR_NAME}" )
