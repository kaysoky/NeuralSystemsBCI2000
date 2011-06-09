###########################################################################
## $Id: CMakeLists.txt 3289 2011-05-18 22:13:10Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for the MSAutoConnectWiimote program

IF( ${BUILD_WIIMOTELOGGER} )

  # Set the executable name
  SET( NAME MSAutoConnectWiimote )

  IF( WIN32 )

    MESSAGE( "-- Adding Wiimote Tool: " ${NAME} )

    # Set the project specific sources
    SET( SRC_PROJECT
      MSAutoConnectWiimote.cpp
    )
    SET( HDR_PROJECT
    )

    INCLUDE_DIRECTORIES(
    )

    SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/tools/ )
    ADD_EXECUTABLE( ${NAME} ${SRC_PROJECT} ${HDR_PROJECT} )

  ENDIF( WIN32 )

ENDIF( ${BUILD_WIIMOTELOGGER} )
