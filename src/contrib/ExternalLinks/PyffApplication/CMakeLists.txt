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
SET( SRC_PROJECT
  PyffApplication.cpp
  ${BCPY2000SHARED}/PythonFilter.cpp
  ${BCPY2000SHARED}/PythonLink24.cpp
  ${BCPY2000SHARED}/PythonLink25.cpp
  ${BCPY2000SHARED}/PythonLink26.cpp
  ${BCPY2000SHARED}/PythonWrapper.cpp
)
SET( HDR_PROJECT
  ${BCPY2000SHARED}/PythonFilter.h
  ${BCPY2000SHARED}/PythonWrapper.h
)

ADD_DEFINITIONS(
  -DDYNAMIC_PYTHON=1
  -DFILTER_SUPERCLASS=GenericFilter
  -DFILTER_NAME=PyffFilter
  -DFILTER_ABBREV=\"PyffApp\"
  -DPYTHON_CONSOLE=\"BCI2000Pyff\"
  -DPYTHON_CONSOLE_INSTALLED\"\"
  -DPYTHON_COREMODULE=\"BCI2000Pyff\"
  -DPYTHON_COREMODULE_INSTALLED=\"\"
  -DPYTHON_MODULE=\"BCI2000Pyff\"
  -DPYTHON_MODULE_INSTALLED=\"\"
  -DPYTHON_SUPERCLASS=\"BCI2000PyffAdapter\"
  -DPYTHON_SUBCLASS=\"\"
  -DDEFAULT_CLASS_FILE=\"\"
)


# Create the signal processing module
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
