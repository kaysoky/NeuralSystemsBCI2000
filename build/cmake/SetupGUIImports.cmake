###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Modifies the framework and sets up GUIs to be imported

# Sets up the extlib dependencies by looping through the BCI2000_GUI_IMPORT var
MACRO( BCI2000_SETUP_GUI_IMPORTS SRC_FRAMEWORK HDR_FRAMEWORK )

# Make sure the input is treated as variables
SET( SOURCES "${SRC_FRAMEWORK}" )
SET( HEADERS "${HDR_FRAMEWORK}" )

# We'll loop through the using statements
FOREACH( USE ${BCI2000_GUI_IMPORT} )

  # Include the appropriate GUI definition
  INCLUDE( ${BCI2000_CMAKE_DIR}/gui/${USE}.cmake )
  IF( GUI_OK )
    SET( ${SOURCES}
         ${${SOURCES}}
         ${SRC_GUI}
    )
    SET( ${HEADERS}
         ${${HEADERS}}
         ${HDR_GUI}
    )
    INCLUDE_DIRECTORIES( ${INC_GUI} )
    SET( MOC
         ${MOC}
         ${MOC_GUI}
    )
    SET( UI
         ${UI}
         ${UI_GUI}
    )
  ENDIF( GUI_OK )

  UNSET( SRC_GUI )
  UNSET( HDR_GUI )
  UNSET( MOC_GUI )
  UNSET( UI_GUI )
  UNSET( GUI_OK )

ENDFOREACH( USE )

IF( NOT BORLAND )
# Wrap UI files
QT4_WRAP_UI( UI_SRCS ${UI} )
SOURCE_GROUP( Generated FILES ${UI_SRCS} )
SET( ${SOURCES}
  ${${SOURCES}}
  ${UI_SRCS}
)

# Wrap MOCable sources for this GUI
QT4_WRAP_CPP( MOC_SRCS ${MOC} )
SOURCE_GROUP( Generated FILES ${MOC_SRCS} )
SET( ${SOURCES}
  ${${SOURCES}}
  ${MOC_SRCS}
)
ENDIF( NOT BORLAND )

# Clear out all imports for next project
UNSET( BCI2000_GUI_IMPORT)

ENDMACRO( BCI2000_SETUP_GUI_IMPORTS SRC_FRAMEWORK HDR_FRAMEWORK )

# Add a GUI Import to the list
MACRO( BCI2000_IMPORT_GUI GUI )
SET( BCI2000_GUI_IMPORT 
  ${BCI2000_GUI_IMPORT}
  ${GUI}
)
ENDMACRO( BCI2000_IMPORT_GUI GUI ) 