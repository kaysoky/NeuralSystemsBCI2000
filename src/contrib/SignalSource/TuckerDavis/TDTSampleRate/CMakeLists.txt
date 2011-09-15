###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the TDTSampleRate tool

# Set the executable name
SET( NAME TDTSampleRate )

IF( NOT BORLAND )

MESSAGE( "-- Adding Tool Project: " ${NAME} )

# Set the project specific sources
SET( SRC_PROJECT
  TDTSampleRate.cpp
  MainDialog.cpp
)
SET( HDR_PROJECT
  MainDialog.h
)

QT_WRAP_CPP( ${NAME} GENERATED ${HDR_PROJECT} )
SOURCE_GROUP( Generated FILES ${GENERATED} )
SET( SRC_PROJECT
  ${SRC_PROJECT}
  ${GENERATED}
)

BCI2000_ADD_TO_INVENTORY( Tool ${NAME} )
ADD_EXECUTABLE( ${NAME} WIN32 ${SRC_PROJECT} ${HDR_PROJECT} )
SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DUSE_QT" )

TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} )

ENDIF( NOT BORLAND )
