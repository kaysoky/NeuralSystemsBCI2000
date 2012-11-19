###########################################################################
## $Id: CMakeLists.txt 3497 2011-08-30 16:50:58Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for Neuroscan getparams tool

# Set the executable name
SET( NAME RDAgetparams )

# DEBUG
MESSAGE( "-- Adding Tool Project: " ${NAME} )

# Set the project specific sources
SET( SOURCES
  RDAgetparams.cpp
  ${BCI2000_SRC_DIR}/contrib/SignalSource/RDAClient/RDAProtocol.cpp
)
SET( HEADERS
  ${BCI2000_SRC_DIR}/contrib/SignalSource/RDAClient/RDAProtocol.h
)

# Add in the appropriate error handling module
SET( SRC_SHARED_BCISTREAM 
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream_tool.cpp
)
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES ${SRC_SHARED_BCISTREAM} )
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${SRC_SHARED_BCISTREAM}
)

SET( SOURCES 
  ${SOURCES}
  ${SRC_SHARED_BCISTREAM}
)

SOURCE_GROUP( Headers\\Project FILES ${HEADERS} )
SOURCE_GROUP( Source\\Project FILES ${SOURCES} )

INCLUDE_DIRECTORIES( ${BCI2000_SRC_DIR}/contrib/SignalSource/RDAClient )

# Generate the required framework
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )

BCI2000_ADD_TO_INVENTORY( Tool ${NAME} )
# Add the executable to the project
ADD_EXECUTABLE( ${NAME} ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} ${SOURCES} ${HEADERS} )

# Set the target build folder
SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER ${DIR_NAME} )

# Link against the Qt/VCL Libraries
IF( BORLAND )
  TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} ${LIBS} )
ELSE( BORLAND )
  TARGET_LINK_LIBRARIES( ${NAME} ${LIBS} )
ENDIF( BORLAND )
