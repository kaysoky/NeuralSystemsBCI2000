###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Defines the available application modules for inclusion
##              in a BCI2000 Application - Useful for HID Filters

SET( BCI2000_APPSOURCES
  # Add extra Application modules here
)

SET( BCI2000_APPHEADERS
  # Add extra Application modules here
)

SET( BCI2000_HIDSOURCES
  # Add extra Application modules here
)

SET( BCI2000_HIDHEADERS
  # Add extra Application modules here
)

# Setup the extra signal source modules
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\human_interface_devices FILES ${BCI2000_HIDSOURCES} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application FILES ${BCI2000_APPSOURCES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\human_interface_devices FILES ${BCI2000_HIDHEADERS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application FILES ${BCI2000_APPHEADERS} )

# Update AppSources with new HID Files
SET( BCI2000_APPSOURCES
  ${BCI2000_APPSOURCES}
  ${BCI2000_HIDSOURCES}
)
SET( BCI2000_APPHEADERS
  ${BCI2000_APPHEADERS}
  ${BCI2000_HIDHEADERS}
)

