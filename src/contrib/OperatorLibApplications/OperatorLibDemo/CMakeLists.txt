###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for OperatorCl

# Set the executable name
SET( EXECUTABLE_NAME OperatorLibDemo )

# DEBUG
MESSAGE( "-- Adding Project: ${EXECUTABLE_NAME}" )

# Set the project specific sources
SET( SRC_PROJECT
  main.cpp 
)
SET( HDR_PROJECT
)

# Generate the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/MinimalFramework.cmake )

# Add in the appropriate error handling module
SET( SRC_SHARED_BCISTREAM 
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_tool.cpp
)
SET( SRC_SHARED_UTILS
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.cpp
)
SET( HDR_SHARED_UTILS
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.h
)

SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES ${SRC_SHARED_BCISTREAM} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils FILES ${SRC_SHARED_UTILS} )
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${SRC_SHARED_BCISTREAM}
  ${SRC_SHARED_UTILS}
)
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils FILES ${HDR_SHARED_UTILS} )
SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${HDR_SHARED_UTILS}
)

SET( HDR_PROJECT
     ${HDR_PROJECT}
     ${BCI2000_SRC_DIR}/core/Operator/OperatorLib/BCI_OperatorLib.h
)
INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/core/Operator/OperatorLib )

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SRC_PROJECT} )
SOURCE_GROUP( Headers\\Project FILES ${HDR_PROJECT} )

# Setup Extlib Dependencies
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS )

# Add the executable to the project
ADD_EXECUTABLE( 
  ${EXECUTABLE_NAME} 
  ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} 
  ${SRC_PROJECT} ${HDR_PROJECT} 
)

TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} OperatorLib )

# Link against the Qt Libraries and other dependencies
TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${QT_LIBRARIES} ${LIBS} )

