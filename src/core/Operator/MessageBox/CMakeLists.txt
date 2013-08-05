###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for MessageBox

IF( WIN32 )

# Set the executable name
SET( NAME MessageBox )

SET( SRC_PROJECT
  MessageBox.cpp
  ${BCI2000_SRC_DIR}/core/Operator/OperatorLib/ScriptParser/ParserToken.cpp
  ${BCI2000_SRC_DIR}/shared/types/EncodedString.cpp
)

INCLUDE_DIRECTORIES(
  ${BCI2000_SRC_DIR}/shared/config
)

BCI2000_ADD_TARGET( WINAPP ${NAME} ${SRC_PROJECT} )
SET_OUTPUT_DIRECTORY( "${BCI2000_ROOT_DIR}/prog" ${NAME} )

ENDIF( WIN32 )
