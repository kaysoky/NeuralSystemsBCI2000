###########################################################################
## $Id: SigSrcModule.cmake -1   $
## Authors: griffin.milsap@gmail.com
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Builds a static library containing components specific to
##   source modules.

ADD_SUBDIRECTORY( test )

SET( NAME BCI2000FrameworkSigSrcModule )

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}

  ${PROJECT_SRC_DIR}/shared/fileio/FileWriterBase.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/GenericFileWriter.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/NullFileWriter.cpp

  ${PROJECT_SRC_DIR}/shared/fileio/dat/BCI2000FileWriter.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/dat/BCI2000OutputFormat.cpp

  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/EDFHeader.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/EDFFileWriter.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/EDFOutputBase.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/EDFOutputFormat.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/GDF.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/GDFFileWriter.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/GDFOutputFormat.cpp

  ${PROJECT_SRC_DIR}/shared/modules/signalsource/BufferedADC.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/GenericADC.h
)

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${BCI2000_SIGSRCSOURCES}
  ${BCI2000_SIGSRCSOURCES_LOGGING}
  ${BCI2000_SIGSRCSOURCES_EXTENSIONS}
  ${HDR_BCI2000_FRAMEWORK}
  ${BCI2000_SIGSRCHEADERS}
  ${BCI2000_SIGSRCHEADERS_LOGGING}
  ${BCI2000_SIGSRCHEADERS_EXTENSIONS}
)

# Add link directories
LINK_DIRECTORIES( ${BCI2000_SIGSRCLIBDIRS} )

# Create the library project
UTILS_INCLUDE( frameworks/SigSrcModule )
BCI2000_ADD_REGISTRY( ${REGISTRY_NAME} SRC_BCI2000_FRAMEWORK DEPTARGETS )
BCI2000_ADD_TARGET( INFO "Framework library" STATIC_LIBRARY ${NAME} ${BCI2000_SIGSRCDLLS} )

IF( FAILED )

  MESSAGE( WARNING "Could not satisfy dependencies of the SignalSource library. SignalSource modules will not build." )

ELSE( FAILED )

  TARGET_LINK_LIBRARIES( ${NAME} BCI2000FrameworkCore )
  TARGET_LINK_LIBRARIES( ${NAME} ${BCI2000_SIGSRCLIBS} )
  ADD_DEPENDENCIES( ${NAME} ${DEPTARGETS} )

ENDIF( FAILED )
