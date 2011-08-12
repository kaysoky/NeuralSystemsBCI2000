###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Sets up a module independent BCI2000 Framework of source
##              files and include directories

# Unset all current variables
UNSET( SRC_BCI2000_SHARED_MODULES )
UNSET( HDR_BCI2000_SHARED_MODULES )
UNSET( SRC_BCI2000_SHARED_UTILS )
UNSET( HDR_BCI2000_SHARED_UTILS )
UNSET( SRC_BCI2000_SHARED_UTILS_EXPRESSION )
UNSET( HDR_BCI2000_SHARED_UTILS_EXPRESSION )
UNSET( SRC_BCI2000_SHARED_TYPES )
UNSET( HDR_BCI2000_SHARED_TYPES )
UNSET( SRC_BCI2000_SHARED_ACCESSORS )
UNSET( HDR_BCI2000_SHARED_ACCESSORS )
UNSET( SRC_BCI2000_SHARED_BCISTREAM )
UNSET( HDR_BCI2000_SHARED_BCISTREAM )
UNSET( SRC_BCI2000_SHARED_FILEIO )
UNSET( HDR_BCI2000_SHARED_FILEIO )

# Gather required BCI2000 Framework sources required for the project
# Take care of the Borland VCL conditional first
IF( BORLAND )
SET( SRC_BCI2000_SHARED_MODULES 
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.cpp
  ${BCI2000_SRC_DIR}/shared/modules/CoreModuleVCL.cpp
  ${BCI2000_SRC_DIR}/shared/modules/GenericFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/MessageHandler.cpp
  ${BCI2000_SRC_DIR}/shared/modules/MessageQueue.cpp
)
SET( HDR_BCI2000_SHARED_MODULES 
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.h
  ${BCI2000_SRC_DIR}/shared/modules/CoreModuleVCL.h
  ${BCI2000_SRC_DIR}/shared/modules/GenericFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/MessageHandler.h
  ${BCI2000_SRC_DIR}/shared/modules/MessageQueue.h
)
ELSE( BORLAND )
SET( SRC_BCI2000_SHARED_MODULES 
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.cpp
  ${BCI2000_SRC_DIR}/shared/modules/CoreModuleQT.cpp
  ${BCI2000_SRC_DIR}/shared/modules/GenericFilter.cpp
  ${BCI2000_SRC_DIR}/shared/modules/MessageHandler.cpp
  ${BCI2000_SRC_DIR}/shared/modules/MessageQueue.cpp
)
SET( HDR_BCI2000_SHARED_MODULES 
  ${BCI2000_SRC_DIR}/shared/modules/CoreModule.h
  ${BCI2000_SRC_DIR}/shared/modules/CoreModuleQT.h
  ${BCI2000_SRC_DIR}/shared/modules/GenericFilter.h
  ${BCI2000_SRC_DIR}/shared/modules/MessageHandler.h
  ${BCI2000_SRC_DIR}/shared/modules/MessageQueue.h
)
ENDIF( BORLAND )

# Include everything else
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
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIException.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError_module.cpp
)
SET( HDR_BCI2000_SHARED_BCISTREAM
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIException.h
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIAssert.h
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIError.h
)
SET( SRC_BCI2000_SHARED_FILEIO
  ${BCI2000_SRC_DIR}/shared/fileio/BCIDirectory.cpp
)
SET( HDR_BCI2000_SHARED_FILEIO
  ${BCI2000_SRC_DIR}/shared/fileio/BCIDirectory.h
)

# Dependencies of the framework
IF( WIN32 )
  SET( LIBS ${LIBS} ws2_32 winmm )
ENDIF() 
IF( NOT APPLE AND NOT WIN32 )
  SET( LIBS ${LIBS} rt pthread )
ENDIF()
