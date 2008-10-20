//////////////////////////////////////////////////////////////////////////////////////////////
//
//  File:        NeuroSrv.cpp
//
//  Author:      juergen.mellinger@uni-tuebingen.de
//
//  Date:        Jul 27, 2004
//
//  Description: A class implementing a generic daemon/server for the Neuroscan Acquire
//               protocol.
//
///////////////////////////////////////////////////////////////////////////////////////////////
#include "NeuroSrv.h"

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include "TCPStream.h"

using namespace std;

void
NeuroSrv::CloseConnection( ostream& os )
{
  os.put( EOF );
  TerminateConnection();
}

void
NeuroSrv::SendBasicInfo( ostream& os )
{
  ostringstream oss;
  mBasicInfo.WriteBinary( oss );
  NscPacketHeader( 'DATA', DataType_InfoBlock, InfoType_BasicInfo, oss.str().size() ).WriteBinary( os );
  os.write( oss.str().data(), oss.str().size() ).flush();
}

int
NeuroSrv::Run( int argc, const char* argv[] )
{
  enum
  {
    noError = 0,
    errorOccurred,
  } result = noError;

  typedef void ( NeuroSrv::*MessageAction )( ostream& );
  map<NscPacketHeader, MessageAction> messageActions;
/*messageActions[ NscPacketHeader( 'CTRL', ::GeneralControlCode,::RequestForVersion, 0 ) ]
    = &NeuroSrv::SendVersion;*/
  messageActions[ NscPacketHeader( 'CTRL', ::GeneralControlCode,::ClosingUp, 0 ) ]
    = &NeuroSrv::CloseConnection;
/*messageActions[ NscPacketHeader( 'CTRL', ::ClientControlCode, ::RequestEDFHeader, 0 ) ]
    = &NeuroSrv::SendEDFHeader;*/
/*messageActions[ NscPacketHeader( 'CTRL', ::ClientControlCode, ::RequestAstFile, 0 ) ]
    = &NeuroSrv::SendASTSetupFile;*/
  messageActions[ NscPacketHeader( 'CTRL', ::ClientControlCode, ::RequestBasicInfo, 0 ) ]
    = &NeuroSrv::SendBasicInfo;
  messageActions[ NscPacketHeader( 'CTRL', ::ClientControlCode, ::RequestStartData, 0 ) ]
    = &NeuroSrv::StartSendingData;
  messageActions[ NscPacketHeader( 'CTRL', ::ClientControlCode, ::RequestStopData, 0 ) ]
    = &NeuroSrv::StopSendingData;
  messageActions[ NscPacketHeader( 'CTRL', ::ServerControlCode, ::StartAcquisition, 0 ) ]
    = &NeuroSrv::StartAcquisition;
  messageActions[ NscPacketHeader( 'CTRL', ::ServerControlCode, ::StopAcquisition, 0 ) ]
    = &NeuroSrv::StopAcquisition;

  const char* address = "localhost:4000";
  if( argc > 1 )
    address = argv[ 1 ];

  while( result == noError )
  {
    tcpstream client;
    server_tcpsocket serverSocket( address );
    serverSocket.wait_for_read( tcpsocket::infiniteTimeout );
    client.open( serverSocket );
    mSendingData = false;
    mTerminatingConnection = false;
    NscPacketHeader header;
    while( client && client.is_open() )
    {
      if( mSendingData )
      {
        int timeout = SendData( client );
        serverSocket.wait_for_read( timeout );
      }
      else
        serverSocket.wait_for_read( tcpsocket::infiniteTimeout );

      while( client.rdbuf()->in_avail() && header.ReadBinary( client ) )
      {
        MessageAction action = messageActions[ header ];
        if( action != NULL )
          ( this->*action )( client );
        else
        {
          cerr << argv[ 0 ] << ": received unhandled message(" << header << "), aborting" << endl;
          CloseConnection( client );
          client.close();
          result = errorOccurred;
        }
      }
      if( mTerminatingConnection )
        client.close();
    }
  }
  return result;
}


void
NeuroSrv::Sleep( int inMilliseconds )
{
  ::timeval sleepDuration = { 0, 1000 * inMilliseconds /* convert to microseconds */ };
  ::select( 0, NULL, NULL, NULL, &sleepDuration );
}
