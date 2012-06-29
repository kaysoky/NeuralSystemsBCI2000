###########################################################################
## $Id: CMakeLists.txt 3547 2011-09-15 12:54:10Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for MessageBox

IF( WIN32 )

# Set the executable name
SET( NAME MessageBox )

SET( SRC_PROJECT
  MessageBox.cpp
)
SET( HDR_PROJECT
)

SET( SRC_FRAMEWORK
  ${BCI2000_SRC_DIR}/core/Operator/OperatorLib/ScriptParser/ParserToken.cpp
  ${BCI2000_SRC_DIR}/shared/types/EncodedString.cpp
)

SET( HDR_FRAMEWORK 
  ${BCI2000_SRC_DIR}/core/Operator/OperatorLib/ScriptParser/ParserToken.h
  ${BCI2000_SRC_DIR}/shared/types/EncodedString.h
)

SOURCE_GROUP( Source\ Files\\BCI2000\ Framework FILES ${SRC_FRAMEWORK} )
SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${SRC_FRAMEWORK}
)

SOURCE_GROUP( Header\ Files\\BCI2000\ Framework FILES ${HDR_FRAMEWORK} )
SET( HDR_BCI2000_FRAMEWORK
  ${HRD_BCI2000_FRAMEWORK}
  ${HDR_FRAMEWORK}
)

INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/core/Operator/OperatorLib/ScriptParser
  ${BCI2000_SRC_DIR}/shared/types
  ${BCI2000_SRC_DIR}/shared/config
)

ADD_EXECUTABLE(
  ${NAME}
  WIN32
  ${SRC_BCI2000_FRAMEWORK}
  ${HDR_BCI2000_FRAMEWORK}
  ${SRC_PROJECT}
  ${HDR_PROJECT}
)
SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" ${NAME} )
SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )

ENDIF( WIN32 )
