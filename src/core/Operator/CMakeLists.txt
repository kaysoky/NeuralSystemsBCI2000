###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}Operator/" )

# Recurse down into all project subdirectories
ADD_SUBDIRECTORY( OperatorLib )
IF( NOT BORLAND )
  ADD_SUBDIRECTORY( OperatorQt )
ENDIF( NOT BORLAND )
ADD_SUBDIRECTORY( BCI2000Remote )
ADD_SUBDIRECTORY( BCI2000Shell )
ADD_SUBDIRECTORY( MessageBox )

