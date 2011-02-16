###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000 and CMake

# Recurse down into all project subdirectories
ADD_SUBDIRECTORY( OperatorLib )
IF( NOT BORLAND )
  ADD_SUBDIRECTORY( OperatorQt )
ENDIF( NOT BORLAND )
ADD_SUBDIRECTORY( OperatorCl )
