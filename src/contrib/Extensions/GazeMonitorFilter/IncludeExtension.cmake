###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com

# Add the GazeMonitorFilter to all application modules

SET( BCI2000_APPHEADERS_EXT
   ${BCI2000_APPHEADERS_EXT}
   ${BCI2000_EXTENSION_DIR}/GazeMonitorFilter.h
)

SET( BCI2000_APPSOURCES_EXT
   ${BCI2000_APPSOURCES_EXT}
   ${BCI2000_EXTENSION_DIR}/GazeMonitorFilter.cpp
)
