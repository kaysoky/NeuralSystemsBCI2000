###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Defines the available application components for inclusion
##              in a BCI2000 Application - Useful for HID Filters

SET( BCI2000_APPSOURCES
  ${BCI2000_APPSOURCES}
  ${BCI2000_SRC_DIR}/shared/modules/application/ConnectorFilters.cpp
  # Add extra Application components from the BCI2000 framework here
)

SET( BCI2000_APPHEADERS
  ${BCI2000_APPHEADERS}
  ${BCI2000_SRC_DIR}/shared/modules/application/ConnectorFilters.h
  # Add extra Application components from the BCI2000 framework here
)

SET( BCI2000_HIDSOURCES
  ${BCI2000_HIDSOURCES}
  # Add extra Application HID components here
)

SET( BCI2000_HIDHEADERS
  ${BCI2000_HIDHEADERS}
  # Add extra Application HID components here
)

SET( BCI2000_APPSOURCES_EXT
  ${BCI2000_APPSOURCES_EXT}
  # Add Application Extensions here
)

SET( BCI2000_APPHEADERS_EXT
  ${BCI2000_APPHEADERS_EXT}
  # Add Application Extensions here
)

IF( WIN32 )
  SET( BCI2000_HIDSOURCES
    ${BCI2000_HIDSOURCES}
    ${BCI2000_SRC_DIR}/shared/modules/application/human_interface_devices/KeystrokeFilter.cpp
  )
  SET( BCI2000_HIDHEADERS
    ${BCI2000_HIDHEADERS}
    ${BCI2000_SRC_DIR}/shared/modules/application/human_interface_devices/KeystrokeFilter.h
  )
ENDIF( WIN32 )
