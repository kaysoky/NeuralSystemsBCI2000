###########################################################################
## $Id: BasicFramework.cmake 3482 2011-08-23 17:15:02Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: List of files in a basic BCI2000 framework

SET( SRC_BCI2000_FRAMEWORK
  ${PROJECT_SRC_DIR}/shared/filters/GenericFilter.cpp
  ${PROJECT_SRC_DIR}/shared/filters/ChoiceCombination.cpp
  ${PROJECT_SRC_DIR}/shared/filters/FilterCombination.cpp
  ${PROJECT_SRC_DIR}/shared/filters/StandaloneFilters.cpp
  ${PROJECT_SRC_DIR}/shared/filters/IdentityFilter.h
  ${PROJECT_SRC_DIR}/shared/filters/SubchainFilter.h
  ${PROJECT_SRC_DIR}/shared/filters/SignalStream.h

  ${PROJECT_SRC_DIR}/shared/modules/MessageHandler.cpp
  ${PROJECT_SRC_DIR}/shared/modules/MessageQueue.cpp

  ${PROJECT_SRC_DIR}/shared/utils/LogFile.cpp
  ${PROJECT_SRC_DIR}/shared/utils/RandomGenerator.cpp
  ${PROJECT_SRC_DIR}/shared/utils/MeasurementUnits.cpp
  ${PROJECT_SRC_DIR}/shared/utils/VersionInfo.cpp
  ${PROJECT_SRC_DIR}/shared/utils/EscapedString.cpp
  ${PROJECT_SRC_DIR}/shared/utils/IndexList.cpp
  ${PROJECT_SRC_DIR}/shared/utils/EventQueue.cpp

  ${PROJECT_SRC_DIR}/shared/utils/Expression/ArithmeticExpression.cpp
  ${PROJECT_SRC_DIR}/shared/utils/Expression/Expression.cpp
  ${PROJECT_SRC_DIR}/shared/utils/Expression/ExpressionNodes.cpp
  ${PROJECT_SRC_DIR}/shared/utils/Expression/ExpressionParser.cpp
  ${PROJECT_SRC_DIR}/shared/utils/Expression/ExpressionParser.hpp
  ${PROJECT_SRC_DIR}/shared/utils/Scripting/ScriptingClass.cpp

  ${PROJECT_SRC_DIR}/shared/types/BitmapImage.cpp
  ${PROJECT_SRC_DIR}/shared/types/Blob.cpp
  ${PROJECT_SRC_DIR}/shared/types/Brackets.cpp
  ${PROJECT_SRC_DIR}/shared/types/CfgID.cpp
  ${PROJECT_SRC_DIR}/shared/types/Color.cpp
  ${PROJECT_SRC_DIR}/shared/types/EncodedString.cpp
  ${PROJECT_SRC_DIR}/shared/types/GenericSignal.cpp
  ${PROJECT_SRC_DIR}/shared/types/GenericVisualization.cpp
  ${PROJECT_SRC_DIR}/shared/types/HierarchicalLabel.cpp
  ${PROJECT_SRC_DIR}/shared/types/Label.cpp
  ${PROJECT_SRC_DIR}/shared/types/LabelIndex.cpp
  ${PROJECT_SRC_DIR}/shared/types/Param.cpp
  ${PROJECT_SRC_DIR}/shared/types/ParamList.cpp
  ${PROJECT_SRC_DIR}/shared/types/PhysicalUnit.cpp
  ${PROJECT_SRC_DIR}/shared/types/SignalProperties.cpp
  ${PROJECT_SRC_DIR}/shared/types/SignalType.cpp
  ${PROJECT_SRC_DIR}/shared/types/State.cpp
  ${PROJECT_SRC_DIR}/shared/types/StateList.cpp
  ${PROJECT_SRC_DIR}/shared/types/StateVector.cpp
  ${PROJECT_SRC_DIR}/shared/types/StateVectorSample.cpp
  ${PROJECT_SRC_DIR}/shared/types/Status.cpp
  ${PROJECT_SRC_DIR}/shared/types/SysCommand.cpp
  ${PROJECT_SRC_DIR}/shared/types/VisID.cpp

  ${PROJECT_SRC_DIR}/shared/accessors/BCIEvent.cpp
  ${PROJECT_SRC_DIR}/shared/accessors/Environment.cpp
  ${PROJECT_SRC_DIR}/shared/accessors/ParamRef.cpp
  ${PROJECT_SRC_DIR}/shared/accessors/StateRef.h  

  ${PROJECT_SRC_DIR}/shared/bcistream/BCIStream.cpp
  ${PROJECT_SRC_DIR}/shared/bcistream/BCITest.cpp
  ${PROJECT_SRC_DIR}/shared/bcistream/BCIException.cpp
  ${PROJECT_SRC_DIR}/shared/bcistream/BCIAssert.h

  ${PROJECT_SRC_DIR}/shared/fileio/RunManager.cpp

  ${PROJECT_SRC_DIR}/shared/fileio/dat/BCI2000FileReader.cpp

  ${PROJECT_SRC_DIR}/extlib/math/FastConv.h

  ${PROJECT_SRC_DIR}/shared/utils/Clock.cpp
  ${PROJECT_SRC_DIR}/shared/utils/ClassName.cpp
  ${PROJECT_SRC_DIR}/shared/utils/PrecisionTime.cpp
  ${PROJECT_SRC_DIR}/shared/utils/LCRandomGenerator.cpp
  ${PROJECT_SRC_DIR}/shared/utils/SockStream.cpp
  ${PROJECT_SRC_DIR}/shared/utils/OSError.cpp
  ${PROJECT_SRC_DIR}/shared/utils/OSMutex.cpp
  ${PROJECT_SRC_DIR}/shared/utils/OSSemaphore.cpp
  ${PROJECT_SRC_DIR}/shared/utils/OSSharedMemory.cpp
  ${PROJECT_SRC_DIR}/shared/utils/OSEvent.cpp
  ${PROJECT_SRC_DIR}/shared/utils/OSThread.cpp
  ${PROJECT_SRC_DIR}/shared/utils/ThreadUtils.cpp
  ${PROJECT_SRC_DIR}/shared/utils/ProcessUtils.cpp
  ${PROJECT_SRC_DIR}/shared/utils/FileUtils.cpp
  ${PROJECT_SRC_DIR}/shared/utils/StringUtils.cpp  
  ${PROJECT_SRC_DIR}/shared/utils/ExceptionCatcher.cpp
  ${PROJECT_SRC_DIR}/shared/utils/ReusableThread.cpp
  ${PROJECT_SRC_DIR}/shared/utils/WildcardMatch.cpp
  ${PROJECT_SRC_DIR}/shared/utils/EnvVariable.cpp
  ${PROJECT_SRC_DIR}/shared/utils/DylibImports.cpp
  ${PROJECT_SRC_DIR}/shared/utils/Debugging.cpp

  ${PROJECT_SRC_DIR}/shared/utils/FPExceptMask.h
  ${PROJECT_SRC_DIR}/shared/utils/Lockable.h
  ${PROJECT_SRC_DIR}/shared/utils/Uncopyable.h
  ${PROJECT_SRC_DIR}/shared/utils/Runnable.h
  ${PROJECT_SRC_DIR}/shared/utils/SharedPointer.h
  ${PROJECT_SRC_DIR}/shared/utils/LazyArray.h
  ${PROJECT_SRC_DIR}/shared/utils/OSThreadLocal.h
  ${PROJECT_SRC_DIR}/shared/utils/Uuid.h
  ${PROJECT_SRC_DIR}/shared/utils/BinaryData.h
  ${PROJECT_SRC_DIR}/shared/utils/Resource.h
  ${PROJECT_SRC_DIR}/shared/utils/NumericConstants.h
)

IF( WIN32 )
  SET( SRC_BCI2000_FRAMEWORK
    ${SRC_BCI2000_FRAMEWORK}
    ${PROJECT_SRC_DIR}/shared/utils/SerialStream.cpp
  )
ENDIF()