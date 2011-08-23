###########################################################################
## $Id: BasicFramework.cmake 3482 2011-08-23 17:15:02Z mellinger $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
## Description: Defines a static library containing a basic BCI2000 framework

SET( NAME BasicFramework )

MESSAGE( "---Adding Framework Library: ${NAME}" )

# Gather required BCI2000 Framework sources required for the project
SET( SRC_BCI2000_SHARED_MODULES 
  ${BCI2000_SRC_DIR}/shared/modules/GenericFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/MessageHandler.cpp
  ${BCI2000_SRC_DIR}/shared/modules/MessageQueue.cpp
)
SET( HDR_BCI2000_SHARED_MODULES 
  ${BCI2000_SRC_DIR}/shared/modules/GenericFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/MessageHandler.h
  ${BCI2000_SRC_DIR}/shared/modules/MessageQueue.h
)
SET( SRC_BCI2000_SHARED_UTILS 
  ${BCI2000_SRC_DIR}/shared/utils/ClassName.cpp
  ${BCI2000_SRC_DIR}/shared/utils/EventQueue.cpp
  ${BCI2000_SRC_DIR}/shared/utils/MeasurementUnits.cpp
  ${BCI2000_SRC_DIR}/shared/utils/PrecisionTime.cpp
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.cpp
  ${BCI2000_SRC_DIR}/shared/utils/VersionInfo.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSError.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSEvent.cpp
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.cpp
  ${BCI2000_SRC_DIR}/shared/utils/ExceptionCatcher.cpp
)
SET( HDR_BCI2000_SHARED_UTILS 
  ${BCI2000_SRC_DIR}/shared/utils/ClassName.h
  ${BCI2000_SRC_DIR}/shared/utils/EventQueue.h
  ${BCI2000_SRC_DIR}/shared/utils/MeasurementUnits.h
  ${BCI2000_SRC_DIR}/shared/utils/PrecisionTime.h
  ${BCI2000_SRC_DIR}/shared/utils/SockStream.h
  ${BCI2000_SRC_DIR}/shared/utils/VersionInfo.h
  ${BCI2000_SRC_DIR}/shared/utils/OSError.h
  ${BCI2000_SRC_DIR}/shared/utils/OSMutex.h
  ${BCI2000_SRC_DIR}/shared/utils/OSEvent.h
  ${BCI2000_SRC_DIR}/shared/utils/OSThread.h
  ${BCI2000_SRC_DIR}/shared/utils/FPExceptMask.h
  ${BCI2000_SRC_DIR}/shared/utils/Lockable.h
  ${BCI2000_SRC_DIR}/shared/utils/Uncopyable.h
  ${BCI2000_SRC_DIR}/shared/utils/ExceptionCatcher.h
)
SET( SRC_BCI2000_SHARED_UTILS_EXPRESSION  
  ${BCI2000_SRC_DIR}/shared/utils/Expression/ArithmeticExpression.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Expression/Expression.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Expression/ExpressionParser.cpp
)
SET( HDR_BCI2000_SHARED_UTILS_EXPRESSION
  ${BCI2000_SRC_DIR}/shared/utils/Expression/ArithmeticExpression.h
  ${BCI2000_SRC_DIR}/shared/utils/Expression/Expression.h
  ${BCI2000_SRC_DIR}/shared/utils/Expression/ExpressionParser.hpp
)
SET( SRC_BCI2000_SHARED_TYPES
  ${BCI2000_SRC_DIR}/shared/types/BitmapImage.cpp
  ${BCI2000_SRC_DIR}/shared/types/Brackets.cpp
  ${BCI2000_SRC_DIR}/shared/types/CfgID.cpp
  ${BCI2000_SRC_DIR}/shared/types/Color.cpp
  ${BCI2000_SRC_DIR}/shared/types/EncodedString.cpp
  ${BCI2000_SRC_DIR}/shared/types/GenericSignal.cpp
  ${BCI2000_SRC_DIR}/shared/types/GenericVisualization.cpp
  ${BCI2000_SRC_DIR}/shared/types/HierarchicalLabel.cpp
  ${BCI2000_SRC_DIR}/shared/types/Label.cpp
  ${BCI2000_SRC_DIR}/shared/types/LabelIndex.cpp
  ${BCI2000_SRC_DIR}/shared/types/Param.cpp
  ${BCI2000_SRC_DIR}/shared/types/ParamList.cpp
  ${BCI2000_SRC_DIR}/shared/types/PhysicalUnit.cpp
  ${BCI2000_SRC_DIR}/shared/types/SignalProperties.cpp
  ${BCI2000_SRC_DIR}/shared/types/SignalType.cpp
  ${BCI2000_SRC_DIR}/shared/types/State.cpp
  ${BCI2000_SRC_DIR}/shared/types/StateList.cpp
  ${BCI2000_SRC_DIR}/shared/types/StateVector.cpp
  ${BCI2000_SRC_DIR}/shared/types/StateVectorSample.cpp
  ${BCI2000_SRC_DIR}/shared/types/Status.cpp
  ${BCI2000_SRC_DIR}/shared/types/SysCommand.cpp
)
SET( HDR_BCI2000_SHARED_TYPES 
  ${BCI2000_SRC_DIR}/shared/types/BitmapImage.h
  ${BCI2000_SRC_DIR}/shared/types/Brackets.h
  ${BCI2000_SRC_DIR}/shared/types/CfgID.h
  ${BCI2000_SRC_DIR}/shared/types/Color.h
  ${BCI2000_SRC_DIR}/shared/types/EncodedString.h
  ${BCI2000_SRC_DIR}/shared/types/GenericSignal.h
  ${BCI2000_SRC_DIR}/shared/types/GenericVisualization.h
  ${BCI2000_SRC_DIR}/shared/types/HierarchicalLabel.h
  ${BCI2000_SRC_DIR}/shared/types/Label.h
  ${BCI2000_SRC_DIR}/shared/types/LabelIndex.h
  ${BCI2000_SRC_DIR}/shared/types/Param.h
  ${BCI2000_SRC_DIR}/shared/types/ParamList.h
  ${BCI2000_SRC_DIR}/shared/types/PhysicalUnit.h
  ${BCI2000_SRC_DIR}/shared/types/SignalProperties.h
  ${BCI2000_SRC_DIR}/shared/types/SignalType.h
  ${BCI2000_SRC_DIR}/shared/types/State.h
  ${BCI2000_SRC_DIR}/shared/types/StateList.h
  ${BCI2000_SRC_DIR}/shared/types/StateVector.h
  ${BCI2000_SRC_DIR}/shared/types/StateVectorSample.h
  ${BCI2000_SRC_DIR}/shared/types/Status.h
  ${BCI2000_SRC_DIR}/shared/types/SysCommand.h
)
SET( SRC_BCI2000_SHARED_ACCESSORS 
  ${BCI2000_SRC_DIR}/shared/accessors/BCIEvent.cpp
  ${BCI2000_SRC_DIR}/shared/accessors/Environment.cpp
  ${BCI2000_SRC_DIR}/shared/accessors/ParamRef.cpp
)
SET( HDR_BCI2000_SHARED_ACCESSORS 
  ${BCI2000_SRC_DIR}/shared/accessors/BCIEvent.h
  ${BCI2000_SRC_DIR}/shared/accessors/Environment.h
  ${BCI2000_SRC_DIR}/shared/accessors/ParamRef.h
  ${BCI2000_SRC_DIR}/shared/accessors/StateRef.h  
)
SET( SRC_BCI2000_SHARED_BCISTREAM
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIException.cpp
)
SET( HDR_BCI2000_SHARED_BCISTREAM
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError.h
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIException.h
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIAssert.h
)
SET( SRC_BCI2000_SHARED_FILEIO
  ${BCI2000_SRC_DIR}/shared/fileio/BCIDirectory.cpp
)
SET( HDR_BCI2000_SHARED_FILEIO
  ${BCI2000_SRC_DIR}/shared/fileio/BCIDirectory.h
)

SET( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_SHARED_UTILS}
  ${SRC_BCI2000_SHARED_UTILS_EXPRESSION}
  ${SRC_BCI2000_SHARED_MODULES}
  ${SRC_BCI2000_SHARED_TYPES}
  ${SRC_BCI2000_SHARED_ACCESSORS}
  ${SRC_BCI2000_SHARED_BCISTREAM}
  ${SRC_BCI2000_SHARED_FILEIO}
)

SET( HDR_BCI2000_FRAMEWORK
  ${HDR_BCI2000_SHARED_UTILS}
  ${HDR_BCI2000_SHARED_UTILS_EXPRESSION}
  ${HDR_BCI2000_SHARED_MODULES}
  ${HDR_BCI2000_SHARED_TYPES}
  ${HDR_BCI2000_SHARED_ACCESSORS}
  ${HDR_BCI2000_SHARED_BCISTREAM}
  ${HDR_BCI2000_SHARED_FILEIO}
)

# Set the BCI2000 Source Groups
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils FILES ${SRC_BCI2000_SHARED_UTILS} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\utils\\expression FILES ${SRC_BCI2000_SHARED_UTILS_EXPRESSION} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules FILES ${SRC_BCI2000_SHARED_MODULES} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\types FILES ${SRC_BCI2000_SHARED_TYPES} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\accessors FILES ${SRC_BCI2000_SHARED_ACCESSORS} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\bcistream FILES ${SRC_BCI2000_SHARED_BCISTREAM} )
SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\fileio FILES ${SRC_BCI2000_SHARED_FILEIO} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils FILES ${HDR_BCI2000_SHARED_UTILS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\utils\\expression FILES ${HDR_BCI2000_SHARED_UTILS_EXPRESSION} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules FILES ${HDR_BCI2000_SHARED_MODULES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\types FILES ${HDR_BCI2000_SHARED_TYPES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\accessors FILES ${HDR_BCI2000_SHARED_ACCESSORS} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\bcistream FILES ${HDR_BCI2000_SHARED_BCISTREAM} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\fileio FILES ${HDR_BCI2000_SHARED_FILEIO} )

# Include "usage header" to define include directories
INCLUDE( ${BCI2000_CMAKE_DIR}/frameworks/BasicFramework.cmake )

ADD_LIBRARY( ${NAME} STATIC ${SRC_BCI2000_FRAMEWORK} ${HDR_BCI2000_FRAMEWORK} )
