###########################################################################
## $Id: SigSrcModule.cmake -1   $
## Authors: griffin.milsap@gmail.com
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Builds a static library containing components specific to
##   source modules.

SET( NAME BCI2000FrameworkSigSrcModule )

MESSAGE( "---Adding Framework Library: ${NAME}" )

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/CoreModuleLib.cmake )

SET( SRC_BCI2000_SHARED_FILEIO
  ${BCI2000_SRC_DIR}/shared/fileio/FileWriterBase.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/GenericFileWriter.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/NullFileWriter.cpp
)
SET( HDR_BCI2000_SHARED_FILEIO
  ${BCI2000_SRC_DIR}/shared/fileio/FileWriterBase.h
  ${BCI2000_SRC_DIR}/shared/fileio/GenericFileWriter.h
  ${BCI2000_SRC_DIR}/shared/fileio/NullFileWriter.h
)
SET( SRC_BCI2000_SHARED_FILEIO_DAT
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000FileWriter.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000OutputFormat.cpp
)
SET( HDR_BCI2000_SHARED_FILEIO_DAT
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000FileWriter.h
  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000OutputFormat.h
)
SET( SRC_BCI2000_SHARED_FILEIO_EDF_GDF
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/EDFFileWriter.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/EDFOutputBase.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/EDFOutputFormat.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/GDF.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/GDFFileWriter.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/GDFOutputFormat.cpp
)
SET( HDR_BCI2000_SHARED_FILEIO_EDF_GDF
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/EDFFileWriter.h
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/EDFOutputBase.h
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/EDFOutputFormat.h
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/GDF.h
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/GDFFileWriter.h
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf/GDFOutputFormat.h
)
SET( SRC_BCI2000_SHARED_MODULES_SIGNALSOURCE
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/BufferedADC.cpp
)
SET( HDR_BCI2000_SHARED_MODULES_SIGNALSOURCE
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/GenericADC.h
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/BufferedADC.h
)

# Set the BCI2000 Source Groups
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\fileio FILES ${SRC_BCI2000_SHARED_FILEIO} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\fileio\\dat FILES ${SRC_BCI2000_SHARED_FILEIO_DAT} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\fileio\\edf_gdf FILES ${SRC_BCI2000_SHARED_FILEIO_EDF_GDF} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\signalsource FILES ${SRC_BCI2000_SHARED_MODULES_SIGNALSOURCE} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\fileio FILES ${HDR_BCI2000_SHARED_FILEIO} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\fileio\\dat FILES ${HDR_BCI2000_SHARED_FILEIO_DAT} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\fileio\\edf_gdf FILES ${HDR_BCI2000_SHARED_FILEIO_EDF_GDF} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\signalsource FILES ${HDR_BCI2000_SHARED_MODULES_SIGNALSOURCE} )

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${SRC_BCI2000_SHARED_FILEIO}
  ${SRC_BCI2000_SHARED_FILEIO_DAT}
  ${SRC_BCI2000_SHARED_FILEIO_EDF_GDF}
  ${SRC_BCI2000_SHARED_MODULES_SIGNALSOURCE}
)

SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${HDR_BCI2000_SHARED_FILEIO}
  ${HDR_BCI2000_SHARED_FILEIO_DAT}
  ${HDR_BCI2000_SHARED_FILEIO_EDF_GDF}
  ${HDR_BCI2000_SHARED_MODULES_SIGNALSOURCE}
)

# Add extensions, loggers, other components
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\signalsource FILES ${BCI2000_SIGSRCSOURCES} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\signalsource\\logging FILES ${BCI2000_SIGSRCSOURCES_LOGGING} )
SOURCE_GROUP( Headers\\Extensions FILES ${BCI2000_SIGSRCHEADERS_EXTENSIONS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\signalsource FILES ${BCI2000_SIGSRCHEADERS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\signalsource\\logging FILES ${BCI2000_SIGSRCHEADERS_LOGGING} )
SOURCE_GROUP( Source\\Extensions FILES ${BCI2000_SIGSRCSOURCES_EXTENSIONS} )

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}
  ${BCI2000_SIGSRCSOURCES}
  ${BCI2000_SIGSRCSOURCES_LOGGING}
  ${BCI2000_SIGSRCSOURCES_EXTENSIONS}
)
SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_FRAMEWORK}
  ${BCI2000_SIGSRCHEADERS}
  ${BCI2000_SIGSRCHEADERS_LOGGING}
  ${BCI2000_SIGSRCHEADERS_EXTENSIONS}
)

# Add link directories
LINK_DIRECTORIES( ${BCI2000_SIGSRCLIBDIRS} )

# Create the library project
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/SigSrcModule.cmake )
BCI2000_ADD_REGISTRY( ${REGISTRY_NAME} SRC_BCI2000_FRAMEWORK DEPTARGETS )
BCI2000_SETUP_EXTLIB_DEPENDENCIES( SRC_BCI2000_FRAMEWORK HDR_BCI2000_FRAMEWORK DEPLIBS FAILED )
IF( FAILED )
  MESSAGE( WARNING "Could not satisfy dependencies of the SignalSource library. SignalSource modules will not build." )
ELSE( FAILED )
  ADD_LIBRARY( ${NAME} STATIC ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} )
  TARGET_LINK_LIBRARIES( ${NAME} ${DEPLIBS} BCI2000FrameworkCore )
  ADD_DEPENDENCIES( ${NAME} ${DEPTARGETS} )
  # Add Pre-processor defines
  IF( NOT BORLAND )
    SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=1 -DUSE_QT" )
  ELSE( NOT BORLAND )
    SET_PROPERTY( TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DMODTYPE=1" )
  ENDIF( NOT BORLAND )
  # Link against the Qt/VCL Libraries
  IF( BORLAND )
    TARGET_LINK_LIBRARIES( ${NAME} vcl rtl ${VXL_VGUI_LIBRARIES} ${BCI2000_SIGSRCLIBS} )
  ELSE( BORLAND )
    TARGET_LINK_LIBRARIES( ${NAME} ${QT_LIBRARIES} ${BCI2000_SIGSRCLIBS} )
  ENDIF( BORLAND )

  # Set the project build folder
  SET_PROPERTY( TARGET ${NAME} PROPERTY FOLDER "${DIR_NAME}" )
ENDIF( FAILED )
# Copy required DLLs
FOREACH( SIGSRCDLL ${BCI2000_SIGSRCDLLS} )
  ADD_CUSTOM_COMMAND(
    TARGET ${NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${SIGSRCDLL}" "${BCI2000_ROOT_DIR}/prog"
  )
ENDFOREACH( SIGSRCDLL )

