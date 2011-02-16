###########################################################################
## $Id$
## Authors: Adam Wilson
## Description: Build information for BCI2000CertAnalysis

# Set the executable name
SET( EXECUTABLE_NAME BCI2000CertAnalysis)

# Set the project specific sources
SET( HDR_PROJECT
  analysisClass.h
  ../Functions.h
  ../TaskType.h
  analysisGUI.h
  certificationClass.h
  ../progressClass.h
)
SET( SRC_PROJECT
  analysisClass.cpp
  analysisMain.cpp
  ../Functions.cpp
  ../TaskType.cpp
  analysisGUI.cpp
  certificationClass.cpp
  ../progressClass.cpp
)

# Wrap the Qt files
QT4_WRAP_CPP( GENERATED_MOC 
  ../progressClass.h
  analysisGUI.h
)
SET( GENERATED
  ${GENERATED_MOC}
)

SET( GENERATED
  ${GENERATED}
  ${GENERATED_RCC}
)

INCLUDE_DIRECTORIES( ../ )

# Create the signal processing module
BCI2000_ADD_TOOLS_GUIAPP( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
  "${GENERATED}"
)


# Set the output directories
SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/BCI2000Certification )
SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/BCI2000Certification )
SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/tools/BCI2000Certification )
IF( MSVC )
  SET_TARGET_PROPERTIES( ${EXECUTABLE_NAME} PROPERTIES 
    PREFIX "../"
  )
ENDIF( MSVC )
