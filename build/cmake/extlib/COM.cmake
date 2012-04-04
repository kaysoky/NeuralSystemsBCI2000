###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including the COM library into
##   a project.
## SETS:
##       SRC_EXTLIB - Required source files
##       HDR_EXTLIB - Required header files
##       INC_EXTLIB - Include directory
##       LIBDIR_EXTLIB - Library directory
##       LIBS_EXTLIB - required libraries
##       Also defines source groups for COM

IF( WIN32 )

# Define the source files
SET( SRC_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/com/ComModule.def
  ${BCI2000_SRC_DIR}/extlib/com/ComModule.cpp
  ${BCI2000_SRC_DIR}/extlib/com/ComClassFactory.cpp
  ${BCI2000_SRC_DIR}/extlib/com/ComRegistrar.cpp
  ${BCI2000_SRC_DIR}/extlib/com/ComStrings.cpp
)

# Define the headers
SET( HDR_EXTLIB
  ${BCI2000_SRC_DIR}/extlib/com/ComModule.h
  ${BCI2000_SRC_DIR}/extlib/com/ComClassFactory.h
  ${BCI2000_SRC_DIR}/extlib/com/ComCoClass.h
  ${BCI2000_SRC_DIR}/extlib/com/ComRegistrar.h
  ${BCI2000_SRC_DIR}/extlib/com/ComStrings.h
  ${BCI2000_SRC_DIR}/extlib/com/ComPtr.h
)

# Define the include directory
SET( INC_EXTLIB ${BCI2000_SRC_DIR}/extlib/com )

# Set the source groups
SOURCE_GROUP( Source\\BCI2000_Framework\\extlib\\com FILES ${SRC_EXTLIB} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\extlib\\com FILES ${HDR_EXTLIB} )

# Set success
SET( EXTLIB_OK TRUE )

ELSE( WIN32 )

  MESSAGE( "- WARNING: COM is only supported on windows." )
  SET( EXTLIB_OK FALSE )

ENDIF( WIN32 )
