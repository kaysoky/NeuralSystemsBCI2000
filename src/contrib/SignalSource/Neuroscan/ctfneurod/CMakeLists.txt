
ADD_EXECUTABLE(
  demoneurod
  demoneurod.cc
  NeuroSrv.cc
  NeuroSrv.h
  SockStream.cc
  SockStream.h
  NeuroscanProtocol.h
)

IF( WIN32 )
  TARGET_LINK_LIBRARIES( demoneurod ws2_32 )
ENDIF( WIN32 )

