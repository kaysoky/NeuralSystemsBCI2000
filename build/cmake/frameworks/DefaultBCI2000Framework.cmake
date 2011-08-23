###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up a module independent BCI2000 Framework of source
##              files and include directories


INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/MinimalFramework.cmake )

# Add CoreModule framework classes
SET( SRC_BCI2000_SHARED_MODULES 
  ${SRC_BCI2000_SHARED_MODULES}
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.cpp
)
SET( HDR_BCI2000_SHARED_MODULES 
  ${HDR_BCI2000_SHARED_MODULES}
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.h
)
SET( SRC_BCI2000_SHARED_BCISTREAM
  ${SRC_BCI2000_SHARED_BCISTREAM}
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

