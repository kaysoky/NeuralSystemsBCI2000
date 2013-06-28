###########################################################################
## $Id$
## Authors: jezhill@gmail.com
## Description: Build information for custom-development bootstrapping tools

SET( DIR_NAME Build/BootstrapCustomProjects )

SET( tools_
  NewBCI2000Module
  NewBCI2000Filter
  NewBCI2000FilterTool
)
FOREACH( tool_ ${tools_} )
  BCI2000_ADD_TARGET(
    INFO "Custom bootstrapping tool"
    EXECUTABLE ${tool_}
    BootstrapCustomProjects.cpp
    ${PROJECT_SRC_DIR}/shared/fileio/dirent_win.h
    OUTPUT_DIRECTORY ${PROJECT_ROOT_DIR}/build
    # That's right: these tools are designed to be within easy reach of the same user who then launches "Make * Project Files.bat"    
  )
ENDFOREACH()

UTILS_ADD_FLAG( NewBCI2000Module "-DMAIN_FUNCTION=NewModule" )
UTILS_ADD_FLAG( NewBCI2000Filter "-DMAIN_FUNCTION=NewFilter" )
UTILS_ADD_FLAG( NewBCI2000FilterTool "-DMAIN_FUNCTION=SetupFilterTool" )
