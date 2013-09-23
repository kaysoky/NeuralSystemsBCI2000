###########################################################################
## $Id: IncludeExtension.cmake 4536 2013-08-05 14:30:13Z mellinger $
## Authors: griffin.milsap@gmail.com

# Add the StimBoxFilter to all application modules

SET( BCI2000_APPHEADERS_EXT
   ${BCI2000_APPHEADERS_EXT}
   ${BCI2000_EXTENSION_DIR}/StimBoxFilter.h
   ${BCI2000_SRC_DIR}/extlib/gtec/gSTIMbox/gSTIMbox.h
   ${BCI2000_SRC_DIR}/extlib/gtec/gSTIMbox/gSTIMbox.imports.h
)

SET( BCI2000_APPSOURCES_EXT
   ${BCI2000_APPSOURCES_EXT}
   ${BCI2000_EXTENSION_DIR}/StimBoxFilter.cpp
   ${BCI2000_SRC_DIR}/extlib/gtec/gSTIMbox/gSTIMbox.imports.cpp
)
