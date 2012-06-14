###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000PythonBindings

# Set the executable name
SET( NAME BCI2000PythonBindings )

# DEBUG
MESSAGE( "-- Adding Project: ${NAME}" )

ADD_CUSTOM_TARGET( ${NAME} )
# Set the project build folder
SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )
ADD_CUSTOM_COMMAND(
  TARGET "${NAME}"
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/BCI2000Remote.py" "${BCI2000_ROOT_DIR}/prog"
)

