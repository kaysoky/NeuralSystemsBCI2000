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
)

SET( HDR_PROJECT
  ParamEnv.h
  Generator.h
  SourceGenerator.h
  SpatialGenerator.h
  ARGenerator.h
  P3Generator.h
)

# Create the Command line module
BCI2000_ADD_TOOLS_CMDLINE( 
  "${NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  FALSE # do not use Qt
)

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build/buildutils/${NAME} ${NAME} )
