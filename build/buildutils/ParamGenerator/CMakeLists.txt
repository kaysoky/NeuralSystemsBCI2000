###########################################################################
## $Id$
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the ParamGenerator tool

SET( NAME ParamGenerator )

# Set the project specific sources
SET( SRC_PROJECT
  ParamEnv.cpp
  Generator.cpp
  SourceGenerator.cpp
  SpatialGenerator.cpp
  ARGenerator.cpp
  P3Generator.cpp
  main.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream_tool.cpp
)
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/Core.cmake )
BCI2000_ADD_TARGET( EXECUTABLE "${NAME}" "${SRC_PROJECT}" )
SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build/buildutils/${NAME} ${NAME} )
