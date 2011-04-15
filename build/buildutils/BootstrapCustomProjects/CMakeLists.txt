###########################################################################
## $Id$
## Authors: jezhill@gmail.com
## Description: Build information for custom-development bootstrapping tools

ADD_EXECUTABLE( NewBCI2000Module NewModuleNewFilter.cpp )
ADD_EXECUTABLE( NewBCI2000Filter NewModuleNewFilter.cpp )

SET_PROPERTY( TARGET NewBCI2000Module APPEND PROPERTY COMPILE_FLAGS "-DMAIN_FUNCTION=NewModule" )
SET_PROPERTY( TARGET NewBCI2000Filter APPEND PROPERTY COMPILE_FLAGS "-DMAIN_FUNCTION=NewFilter" )

SET_TARGET_PROPERTIES( NewBCI2000Module PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/build )
SET_TARGET_PROPERTIES( NewBCI2000Filter PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BCI2000_ROOT_DIR}/build )
# That's right: these tools are designed to be within easy reach of the same user who then launches "Make * Project Files.bat"

IF (MSVC OR XCODE)
   SET_TARGET_PROPERTIES( NewBCI2000Module PROPERTIES PREFIX "../" )
   SET_TARGET_PROPERTIES( NewBCI2000Filter PROPERTIES PREFIX "../" )
ENDIF(MSVC OR XCODE)

MESSAGE( "-- Added custom-development bootstrapping tools" )
