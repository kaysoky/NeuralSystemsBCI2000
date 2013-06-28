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
  ${PROJECT_SRC_DIR}/shared/bcistream/BCIStream_tool.cpp
)
UTILS_INCLUDE( frameworks/Core )
BCI2000_ADD_TARGET( EXECUTABLE "${NAME}" "${SRC_PROJECT}" ${PROJECT_ROOT_DIR}/build/buildutils/${NAME} )
