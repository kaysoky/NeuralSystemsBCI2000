###########################################################################
## $Id: CMakeLists.txt 3497 2011-08-30 16:50:58Z mellinger $
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Build information for PyffApplication

# Set the executable name
SET( EXECUTABLE_NAME PyffApplication )

# Set the project specific sources
SET( BCPY2000SHARED
  ../../BCPy2000/foundation/shared
)
SET( BCI2000REMOTE
  ../../../core/Operator/BCI2000Remote
)
SET( SRC_PROJECT
  PyffApplication.cpp
  PyffFilter.cpp
  ${BCPY2000SHARED}/PythonLink24.cpp
  ${BCPY2000SHARED}/PythonLink25.cpp
  ${BCPY2000SHARED}/PythonLink26.cpp
  ${BCPY2000SHARED}/PythonLink27.cpp
  ${BCPY2000SHARED}/PythonWrapper.cpp
  ${BCI2000REMOTE}/BCI2000Connection.cpp
)
SET( HDR_PROJECT
  PyffFilter.h
  ${BCPY2000SHARED}/PythonWrapper.h
  ${BCI2000REMOTE}/BCI2000Connection.h
)

ADD_DEFINITIONS(
)

INCLUDE_DIRECTORIES(
  ${BCPY2000SHARED}
  ${BCI2000REMOTE}
)

# Create the signal processing module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
