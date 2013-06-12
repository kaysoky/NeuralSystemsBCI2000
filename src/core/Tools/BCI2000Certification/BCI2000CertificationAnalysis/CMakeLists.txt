###########################################################################
## $Id$
## Authors: Adam Wilson
## Description: Build information for BCI2000CertAnalysis

IF( BUILD_TESTS )

# Set the executable name
SET( NAME BCI2000CertAnalysis)

# Set the project specific sources
SET( SOURCES
  analysisClass.cpp
  analysisMain.cpp
  ../Functions.cpp
  ../TaskType.cpp
  analysisGUI.cpp
  certificationClass.cpp
  ../progressClass.cpp
)
SET( DIR_NAME Tests/Certification )
BCI2000_ADD_TOOLS_GUIAPP( ${NAME} ${SOURCES} )
SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/tools/BCI2000Certification ${NAME} )

ENDIF( BUILD_TESTS )
