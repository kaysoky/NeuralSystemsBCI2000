###########################################################################
## $Id: BasicFramework.cmake 3482 2011-08-23 17:15:02Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: List of files in a basic BCI2000 framework

SET( SRC_BCI2000_FRAMEWORK
  ${BCI2000_SRC_DIR}/shared/filters/GenericFilter.cpp
  ${BCI2000_SRC_DIR}/shared/filters/ChoiceCombination.cpp
  ${BCI2000_SRC_DIR}/shared/filters/StandaloneFilters.cpp
  ${BCI2000_SRC_DIR}/shared/filters/IdentityFilter.h
  ${BCI2000_SRC_DIR}/shared/filters/FilterCombination.h

  ${BCI2000_SRC_DIR}/shared/modules/MessageHandler.cpp
  ${BCI2000_SRC_DIR}/shared/modules/MessageQueue.cpp

  ${BCI2000_SRC_DIR}/shared/utils/LogFile.cpp
  ${BCI2000_SRC_DIR}/shared/utils/RandomGenerator.cpp
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
  ${BCI2000_SRC_DIR}/shared/utils/ThreadUtils.cpp
  ${BCI2000_SRC_DIR}/shared/utils/ProcessUtils.cpp
  ${BCI2000_SRC_DIR}/shared/utils/FileUtils.cpp
  ${BCI2000_SRC_DIR}/shared/utils/StringUtils.cpp  
  ${BCI2000_SRC_DIR}/shared/utils/ExceptionCatcher.cpp
  ${BCI2000_SRC_DIR}/shared/utils/ReusableThread.cpp
  ${BCI2000_SRC_DIR}/shared/utils/WildcardMatch.cpp
  ${BCI2000_SRC_DIR}/shared/utils/EscapedString.cpp
  ${BCI2000_SRC_DIR}/shared/utils/EnvVariable.cpp
  ${BCI2000_SRC_DIR}/shared/utils/DylibImports.cpp
  ${BCI2000_SRC_DIR}/shared/utils/IndexList.cpp

  ${BCI2000_SRC_DIR}/shared/utils/FPExceptMask.h
  ${BCI2000_SRC_DIR}/shared/utils/Lockable.h
  ${BCI2000_SRC_DIR}/shared/utils/Uncopyable.h
  ${BCI2000_SRC_DIR}/shared/utils/Runnable.h
  ${BCI2000_SRC_DIR}/shared/utils/SharedPointer.h
  ${BCI2000_SRC_DIR}/shared/utils/OSThreadLocal.h
  ${BCI2000_SRC_DIR}/shared/utils/Uuid.h
  ${BCI2000_SRC_DIR}/shared/utils/BinaryData.h
  ${BCI2000_SRC_DIR}/shared/utils/Resource.h

  ${BCI2000_SRC_DIR}/shared/utils/Expression/ArithmeticExpression.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Expression/Expression.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Expression/ExpressionNodes.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Expression/ExpressionParser.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Expression/ExpressionParser.hpp

  ${BCI2000_SRC_DIR}/shared/types/BitmapImage.cpp
  ${BCI2000_SRC_DIR}/shared/types/Blob.cpp
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
  ${BCI2000_SRC_DIR}/shared/types/VisID.cpp

  ${BCI2000_SRC_DIR}/shared/accessors/BCIEvent.cpp
  ${BCI2000_SRC_DIR}/shared/accessors/Environment.cpp
  ${BCI2000_SRC_DIR}/shared/accessors/ParamRef.cpp
  ${BCI2000_SRC_DIR}/shared/accessors/StateRef.h  

  ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCITest.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIException.h
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIAssert.h

  ${BCI2000_SRC_DIR}/shared/fileio/RunManager.cpp
  ${BCI2000_SRC_DIR}/shared/fileio/RunManager.h

  ${BCI2000_SRC_DIR}/shared/fileio/dat/BCI2000FileReader.cpp
)
