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
BCI2000_USE( "GUSBAMP" )

# Create the signal source module
ADD_EXECUTABLE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${INC_EXTLIB}"
)

# Set the output directories
SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/prog )
IF( MSVC OR XCODE )
  SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES 
    PREFIX "../"
    IMPORT_PREFIX "../" 
  )
ENDIF( MSVC OR XCODE )

TARGET_LINK_LIBRARIES( ${EXECUTABLE_NAME} ${LIBS} )
