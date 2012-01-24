###########################################################################
## $Id:  $
## Authors:
## Description: Build information for the StatisticsSignalProcessing project
##              
##              
###########################################################################

# Set the executable name
SET( EXECUTABLE_NAME StatisticsSignalProcessing )

BCI2000_INCLUDE( "STATISTICS" )

SET( DIR_STAT
  ${BCI2000_SRC_DIR}/shared/modules/signalprocessing/statistics
)
INCLUDE_DIRECTORIES( ${DIR_STAT} )

SET( SRC_STAT
  ${DIR_STAT}/ChannelSetSource.cpp
  ${DIR_STAT}/DataSource.cpp
  ${DIR_STAT}/ExpressionSource.cpp
  ${DIR_STAT}/FunctionSource.cpp
  ${DIR_STAT}/ObserverSource.cpp
  ${DIR_STAT}/ViewSource.cpp
  ${DIR_STAT}/StatisticsFilter.cpp
)

SET( HDR_STAT
  ${DIR_STAT}/ChannelSetSource.h
  ${DIR_STAT}/DataSource.h
  ${DIR_STAT}/ExpressionSource.h
  ${DIR_STAT}/FunctionSource.h
  ${DIR_STAT}/ObserverSource.h
  ${DIR_STAT}/ViewSource.h
  ${DIR_STAT}/StatisticsFilter.h
)

SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\modules\\signalprocessing\\statistics FILES ${SIGPROCSOURCES} )
SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\modules\\signalprocessing\\statistics FILES ${SIGPROCHEADERS} )


# Set the project specific sources
SET( SRC_PROJECT
  StatisticsSignalProcessing.cpp 
  PipeDefinition.cpp
  ${SRC_STAT}
)

SET( HDR_PROJECT
  ${HDR_STAT}
)

BCI2000_ADD_SIGNAL_PROCESSING_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
