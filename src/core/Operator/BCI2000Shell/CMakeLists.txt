###########################################################################
## $Id: CMakeLists.txt 3547 2011-09-15 12:54:10Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000Shell

# Set the executable name
SET( NAME BCI2000Shell )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )

SET( SRC_PROJECT
  main.cpp
  ${BCI2000_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp
)

BCI2000_ADD_TARGET( EXECUTABLE ${NAME} ${SRC_PROJECT} )
SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" ${NAME} )
