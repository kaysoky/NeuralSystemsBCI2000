###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up a module independent BCI2000 Framework of source
##              files and include directories

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/DefaultBCI2000Framework.cmake )

SET( SRC_BCI2000_SHARED_UTILS
  ${SRC_BCI2000_SHARED_UTILS} 
  ${BCI2000_SRC_DIR}/shared/utils/LogFile.cpp
  ${BCI2000_SRC_DIR}/shared/utils/RandomGenerator.cpp
)
SET( HDR_BCI2000_SHARED_UTILS 
  ${HDR_BCI2000_SHARED_UTILS}
  ${BCI2000_SRC_DIR}/shared/utils/LogFile.h
  ${BCI2000_SRC_DIR}/shared/utils/RandomGenerator.h
)

SET( SRC_BCI2000_SHARED_GUI 
  ${BCI2000_SRC_DIR}/shared/gui/GraphObject.cpp
  ${BCI2000_SRC_DIR}/shared/gui/DisplayWindow.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GraphDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/gui/TextField.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GUI.cpp
  ${BCI2000_SRC_DIR}/shared/gui/Shapes.cpp
)
SET( HDR_BCI2000_SHARED_GUI
  ${BCI2000_SRC_DIR}/shared/gui/GraphObject.h
  ${BCI2000_SRC_DIR}/shared/gui/DisplayWindow.h
  ${BCI2000_SRC_DIR}/shared/gui/GraphDisplay.h
  ${BCI2000_SRC_DIR}/shared/gui/TextField.h
  ${BCI2000_SRC_DIR}/shared/gui/GUI.h
  ${BCI2000_SRC_DIR}/shared/gui/Shapes.h
)
SET( SRC_BCI2000_SHARED_MODULES_APPLICATION
  ${BCI2000_SRC_DIR}/shared/modules/application/ApplicationBase.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/ConnectorFilters.cpp
)
SET( HDR_BCI2000_SHARED_MODULES_APPLICATION
  ${BCI2000_SRC_DIR}/shared/modules/application/ApplicationBase.h
  ${BCI2000_SRC_DIR}/shared/modules/application/ConnectorFilters.h
)
SET( SRC_BCI2000_SHARED_MODULES_APPLICATION_UTILS
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/TrialStatistics.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/BlockRandSeq.cpp
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/Localization.cpp
)
SET( HDR_BCI2000_SHARED_MODULES_APPLICATION_UTILS
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/TrialStatistics.h
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/BlockRandSeq.h
  ${BCI2000_SRC_DIR}/shared/modules/application/utils/Localization.h
)

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_SHARED_UTILS}
  ${SRC_BCI2000_SHARED_UTILS_EXPRESSION}
  ${SRC_BCI2000_SHARED_MODULES}
  ${SRC_BCI2000_SHARED_TYPES}
  ${SRC_BCI2000_SHARED_ACCESSORS}
  ${SRC_BCI2000_SHARED_BCISTREAM}
  ${SRC_BCI2000_SHARED_FILEIO}
  ${SRC_BCI2000_SHARED_GUI}
  ${SRC_BCI2000_SHARED_MODULES_APPLICATION}
  ${SRC_BCI2000_SHARED_MODULES_APPLICATION_UTILS}
)

SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_SHARED_UTILS}
  ${HDR_BCI2000_SHARED_UTILS_EXPRESSION}
  ${HDR_BCI2000_SHARED_MODULES}
  ${HDR_BCI2000_SHARED_TYPES}
  ${HDR_BCI2000_SHARED_ACCESSORS}
  ${HDR_BCI2000_SHARED_BCISTREAM}
  ${HDR_BCI2000_SHARED_FILEIO}
  ${HDR_BCI2000_SHARED_GUI}
  ${HDR_BCI2000_SHARED_MODULES_APPLICATION}
  ${HDR_BCI2000_SHARED_MODULES_APPLICATION_UTILS}
)

# Wrap files which need MOCing
SET( HDR_BCI2000_MOC 
  ${BCI2000_SRC_DIR}/shared/gui/DisplayWindow.h
)

# Set the BCI2000 Source Groups
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils FILES ${SRC_BCI2000_SHARED_UTILS} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils\\expression FILES ${SRC_BCI2000_SHARED_UTILS_EXPRESSION} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules FILES ${SRC_BCI2000_SHARED_MODULES} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\types FILES ${SRC_BCI2000_SHARED_TYPES} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\accessors FILES ${SRC_BCI2000_SHARED_ACCESSORS} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES ${SRC_BCI2000_SHARED_BCISTREAM} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\fileio FILES ${SRC_BCI2000_SHARED_FILEIO} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\gui FILES ${SRC_BCI2000_SHARED_GUI} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application FILES ${SRC_BCI2000_SHARED_MODULES_APPLICATION} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\application\\utils FILES ${SRC_BCI2000_SHARED_MODULES_APPLICATION_UTILS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils FILES ${HDR_BCI2000_SHARED_UTILS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils\\expression FILES ${HDR_BCI2000_SHARED_UTILS_EXPRESSION} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules FILES ${HDR_BCI2000_SHARED_MODULES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\types FILES ${HDR_BCI2000_SHARED_TYPES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\accessors FILES ${HDR_BCI2000_SHARED_ACCESSORS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\bcistream FILES ${HDR_BCI2000_SHARED_BCISTREAM} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\fileio FILES ${HDR_BCI2000_SHARED_FILEIO} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\gui FILES ${HDR_BCI2000_SHARED_GUI} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application FILES ${HDR_BCI2000_SHARED_MODULES_APPLICATION} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\application\\utils FILES ${HDR_BCI2000_SHARED_MODULES_APPLICATION_UTILS} )

# Define include directories
IF( BORLAND )
INCLUDE_DIRECTORIES(
  ${VXLCORE_INCLUDE_DIR}
  ${BCI2000_SRC_DIR}/shared
  ${BCI2000_SRC_DIR}/shared/accessors
  ${BCI2000_SRC_DIR}/shared/bcistream
  ${BCI2000_SRC_DIR}/shared/config
  ${BCI2000_SRC_DIR}/shared/modules
  ${BCI2000_SRC_DIR}/shared/types
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/shared/utils/Expression
  ${BCI2000_SRC_DIR}/shared/fileio
  ${BCI2000_SRC_DIR}/shared/gui
  ${BCI2000_SRC_DIR}/shared/modules/application
  ${BCI2000_SRC_DIR}/shared/modules/application/utils
  ${BCI2000_SRC_DIR}/shared/modules/application/audio
  ${BCI2000_SRC_DIR}/shared/modules/application/gui
  ${BCI2000_SRC_DIR}/shared/modules/application/human_interface_devices
  ${BCI2000_SRC_DIR}/shared/modules/application/speller
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli
)
ELSE( BORLAND )
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR} 
  ${BCI2000_SRC_DIR}/shared
  ${BCI2000_SRC_DIR}/shared/accessors
  ${BCI2000_SRC_DIR}/shared/bcistream
  ${BCI2000_SRC_DIR}/shared/config
  ${BCI2000_SRC_DIR}/shared/modules
  ${BCI2000_SRC_DIR}/shared/types
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/shared/utils/Expression
  ${BCI2000_SRC_DIR}/shared/fileio
  ${BCI2000_SRC_DIR}/shared/gui
  ${BCI2000_SRC_DIR}/shared/modules/application
  ${BCI2000_SRC_DIR}/shared/modules/application/utils
  ${BCI2000_SRC_DIR}/shared/modules/application/audio
  ${BCI2000_SRC_DIR}/shared/modules/application/gui
  ${BCI2000_SRC_DIR}/shared/modules/application/human_interface_devices
  ${BCI2000_SRC_DIR}/shared/modules/application/speller
  ${BCI2000_SRC_DIR}/shared/modules/application/stimuli
)
ENDIF( BORLAND )
