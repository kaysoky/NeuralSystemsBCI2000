###########################################################################
## $Id: IncludeExtensionMacro.cmake 3201 2011-04-13 16:03:27Z mellinger $
## Authors: jezhill@gmail.com
## Description: Contains a macro for including optional extensions to the BCI2000 framework
##              (for example, loggers which will by incorporated into every SignalSource module)
##              gated by environment variables.

MACRO( INCLUDE_EXTENSION NAME DIR)
  STRING( TOUPPER ${NAME} UPPERNAME )
  SET( SETTINGNAME "BUILD_${UPPERNAME}" )
  SET( ${SETTINGNAME} FALSE CACHE BOOL "Whether to augment the BCI2000 Framework using the contributed ${NAME} extension"  )
  SET( SETTINGVAL  ${${SETTINGNAME}} )
  SET( BCI2000_EXTENSION_DIR "${DIR}" )
  IF( ${SETTINGVAL} )
    MESSAGE( "-- Including custom extension ${NAME}" )
    MESSAGE( "---- (to disable, set the ${SETTINGNAME} setting to FALSE)" )
    INCLUDE( "${BCI2000_EXTENSION_DIR}/IncludeExtension.cmake" )
  ELSE()
    MESSAGE( "-- Skipping custom extension ${NAME}" )
    MESSAGE( "---- (to enable, set the ${SETTINGNAME} setting to TRUE)" )
  ENDIF( ${SETTINGVAL} )
ENDMACRO( INCLUDE_EXTENSION NAME DIR)
