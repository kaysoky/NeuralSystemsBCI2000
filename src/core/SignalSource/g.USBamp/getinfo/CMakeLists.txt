###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Build information for USBampgetinfo

# Set the executable name
SET( EXECUTABLE_NAME gUSBampgetinfo )

# DEBUG
MESSAGE( "-- Adding Tool Project: " ${EXECUTABLE_NAME} )

# Set the project specific sources
SET( SRC_PROJECT
  gUSBampgetinfo.cpp
)
SET( HDR_PROJECT
)

# Tell the macro to link against the gUSBamp API found in EXTLIB
BCI2000_INCLUDE( "GUSBAMP" )

# Add in external required libraries
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK LIBS FAILED )

IF( NOT FAILED )
  BCI2000_ADD_TO_INVENTORY( Tool ${EXECUTABLE_NAME} )
  
  # Create the signal source module
  ADD_EXECUTABLE( 
    "${EXECUTABLE_NAME}" 
    "${SRC_PROJECT}" "${HDR_PROJECT}" 
    "${SRC_BCI2000_FRAMEWORK}"
    "${HDR_BCI2000_FRAMEWORK}" 
  )
  TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${LIBS} )
  
  # Set the project build folder
  SET_PROPERTY( TARGET ${EXECUTABLE_NAME} PROPERTY FOLDER "${DIR_NAME}" )
ENDIF( NOT FAILED )

