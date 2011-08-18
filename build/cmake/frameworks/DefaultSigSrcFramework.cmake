###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up a module independent BCI2000 Framework of source
##              files and include directories

INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/DefaultBCI2000Framework.cmake )

SET( SRC_BCI2000_SHARED_UTILS 
  ${SRC_BCI2000_SHARED_UTILS}
  ${BCI2000_SRC_DIR}/shared/utils/RandomGenerator.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.cpp
)
SET( HDR_BCI2000_SHARED_UTILS 
  ${HDR_BCI2000_SHARED_UTILS}
  ${BCI2000_SRC_DIR}/shared/utils/RandomGenerator.h
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.h
)
SET( SRC_BCI2000_SHARED_FILEIO
  ${SRC_BCI2000_SHARED_FILEIO}
  ${BCI2000_SRC_DIR}/shared/fileio/FileWriterBase.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/GenericFileWriter.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/NullFileWriter.cpp
)
SET( HDR_BCI2000_SHARED_FILEIO
  ${HDR_BCI2000_SHARED_FILEIO}
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
  ${BCI2000_SRC_DIR}/shared/modules/signalsource/BufferedADC.h
)

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_SHARED_UTILS}
  ${SRC_BCI2000_SHARED_UTILS_EXPRESSION}
  ${SRC_BCI2000_SHARED_MODULES}
  ${SRC_BCI2000_SHARED_TYPES}
  ${SRC_BCI2000_SHARED_ACCESSORS}
  ${SRC_BCI2000_SHARED_BCISTREAM}
  ${SRC_BCI2000_SHARED_FILEIO}
  ${SRC_BCI2000_SHARED_FILEIO_DAT}
  ${SRC_BCI2000_SHARED_FILEIO_EDF_GDF}
  ${SRC_BCI2000_SHARED_MODULES_SIGNALSOURCE}
)

SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_SHARED_UTILS}
  ${HDR_BCI2000_SHARED_UTILS_EXPRESSION}
  ${HDR_BCI2000_SHARED_MODULES}
  ${HDR_BCI2000_SHARED_TYPES}
  ${HDR_BCI2000_SHARED_ACCESSORS}
  ${HDR_BCI2000_SHARED_BCISTREAM}
  ${HDR_BCI2000_SHARED_FILEIO}
  ${HDR_BCI2000_SHARED_FILEIO_DAT}
  ${HDR_BCI2000_SHARED_FILEIO_EDF_GDF}
  ${HDR_BCI2000_SHARED_MODULES_SIGNALSOURCE}
)

# Set the BCI2000 Source Groups
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils FILES ${SRC_BCI2000_SHARED_UTILS} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils\\expression FILES ${SRC_BCI2000_SHARED_UTILS_EXPRESSION} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules FILES ${SRC_BCI2000_SHARED_MODULES} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\types FILES ${SRC_BCI2000_SHARED_TYPES} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\accessors FILES ${SRC_BCI2000_SHARED_ACCESSORS} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES ${SRC_BCI2000_SHARED_BCISTREAM} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\fileio FILES ${SRC_BCI2000_SHARED_FILEIO} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\fileio\\dat FILES ${SRC_BCI2000_SHARED_FILEIO_DAT} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\fileio\\edf_gdf FILES ${SRC_BCI2000_SHARED_FILEIO_EDF_GDF} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\signalsource FILES ${SRC_BCI2000_SHARED_MODULES_SIGNALSOURCE} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils FILES ${HDR_BCI2000_SHARED_UTILS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils\\expression FILES ${HDR_BCI2000_SHARED_UTILS_EXPRESSION} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules FILES ${HDR_BCI2000_SHARED_MODULES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\types FILES ${HDR_BCI2000_SHARED_TYPES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\accessors FILES ${HDR_BCI2000_SHARED_ACCESSORS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\bcistream FILES ${HDR_BCI2000_SHARED_BCISTREAM} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\fileio FILES ${HDR_BCI2000_SHARED_FILEIO} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\fileio\\dat FILES ${HDR_BCI2000_SHARED_FILEIO_DAT} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\fileio\\edf_gdf FILES ${HDR_BCI2000_SHARED_FILEIO_EDF_GDF} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\signalsource FILES ${HDR_BCI2000_SHARED_MODULES_SIGNALSOURCE} )

# Define include directories
IF( BORLAND )
INCLUDE_DIRECTORIES(
  ${VXLCORE_INCLUDE_DIR}
  ${BCI2000_SRC_DIR}/shared
  ${BCI2000_SRC_DIR}/shared/accessors
  ${BCI2000_SRC_DIR}/shared/bcistream
  ${BCI2000_SRC_DIR}/shared/config
  ${BCI2000_SRC_DIR}/shared/modules
  ${BCI2000_SRC_DIR}/shared/types
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/shared/utils/Expression
  ${BCI2000_SRC_DIR}/shared/fileio
  ${BCI2000_SRC_DIR}/shared/fileio/dat
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf
  ${BCI2000_SRC_DIR}/shared/modules/signalsource
)
ELSE( BORLAND )
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR} 
  ${BCI2000_SRC_DIR}/shared
  ${BCI2000_SRC_DIR}/shared/accessors
  ${BCI2000_SRC_DIR}/shared/bcistream
  ${BCI2000_SRC_DIR}/shared/config
  ${BCI2000_SRC_DIR}/shared/modules
  ${BCI2000_SRC_DIR}/shared/types
  ${BCI2000_SRC_DIR}/shared/utils
  ${BCI2000_SRC_DIR}/shared/utils/Expression
  ${BCI2000_SRC_DIR}/shared/fileio
  ${BCI2000_SRC_DIR}/shared/fileio/dat
  ${BCI2000_SRC_DIR}/shared/fileio/edf_gdf
  ${BCI2000_SRC_DIR}/shared/modules/signalsource
)
ENDIF( BORLAND )
