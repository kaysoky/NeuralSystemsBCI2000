###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Source files common to all Core Module libraries

# Add CoreModule framework classes
SET( SRC_BCI2000_SHARED_MODULES 
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.cpp
)
SET( HDR_BCI2000_SHARED_MODULES 
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.h
)
SET( SRC_BCI2000_SHARED_BCISTREAM
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_module.cpp
)

IF( BORLAND )
  SET( SRC_BCI2000_SHARED_MODULES 
    ${SRC_BCI2000_SHARED_MODULES}
	${BCI2000_SRC_DIR}/shared/modules/CoreModuleVCL.cpp
  )
  SET( HDR_BCI2000_SHARED_MODULES 
    ${HDR_BCI2000_SHARED_MODULES}
	${BCI2000_SRC_DIR}/shared/modules/CoreModuleVCL.h
  )
ELSE( BORLAND )
  SET( SRC_BCI2000_SHARED_MODULES 
    ${SRC_BCI2000_SHARED_MODULES}
	${BCI2000_SRC_DIR}/shared/modules/CoreModuleQT.cpp
  )
  SET( HDR_BCI2000_SHARED_MODULES 
    ${HDR_BCI2000_SHARED_MODULES}
	${BCI2000_SRC_DIR}/shared/modules/CoreModuleQT.h
  )
ENDIF( BORLAND )

SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules FILES ${SRC_BCI2000_SHARED_MODULES} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES ${SRC_BCI2000_SHARED_BCISTREAM} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules FILES ${HDR_BCI2000_SHARED_MODULES} )

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_SHARED_MODULES}
  ${SRC_BCI2000_SHARED_BCISTREAM}
)

SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_SHARED_MODULES}
  ${HDR_BCI2000_SHARED_BCISTREAM}
)


