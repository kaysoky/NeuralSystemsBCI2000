###########################################################################
## $Id$
## Authors: jezhill@gmail.com
## Description: Build information for custom-development bootstrapping tools

INCLUDE_DIRECTORIES( "${BCI2000_SRC_DIR}/shared/fileio" )

SET_OUTPUT_DIRECTORY( ${BCI2000_ROOT_DIR}/build )
# That's right: these tools are designed to be within easy reach of the same user who then launches "Make * Project Files.bat"

ADD_EXECUTABLE( NewBCI2000Module BootstrapCustomProjects.cpp ${BCI2000_SRC_DIR}/shared/fileio/dirent_win.h )
ADD_EXECUTABLE( NewBCI2000Filter BootstrapCustomProjects.cpp ${BCI2000_SRC_DIR}/shared/fileio/dirent_win.h )

SET_PROPERTY( TARGET NewBCI2000Module APPEND PROPERTY COMPILE_FLAGS "-DMAIN_FUNCTION=NewModule" )
SET_PROPERTY( TARGET NewBCI2000Filter APPEND PROPERTY COMPILE_FLAGS "-DMAIN_FUNCTION=NewFilter" )


MESSAGE( "-- Added Custom-Development Bootstrapping Tools" )
