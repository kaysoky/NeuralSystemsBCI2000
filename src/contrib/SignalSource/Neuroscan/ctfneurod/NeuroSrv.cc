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
#include <iomanip>
#include <sstream>
#include <map>
#include <cstdio>
#include <stdexcept>

#include "SockStream.h"

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

void
NeuroSrv::SendEDFHeader( std::ostream& os )
{
  ostringstream oss;
  oss.put( '0' );
  for( int i = 1; i < 256 - 4; ++i )
    oss.put( ' ' );
  ostringstream oss2;
  oss2 << setw( 4 ) << mBasicInfo.EEGChannels();
  oss << oss2.str().substr( 0, 4 );

  const struct { string ChannelInfo::* s; size_t l; } fields[] =
  { // EDF fields in a channel entry, and their lengths
    { &ChannelInfo::name, 16 },
    { &ChannelInfo::type, 80 },
    { &ChannelInfo::unit, 8 },
    { NULL, 8 }, { NULL, 8 }, { NULL, 8 }, { NULL, 8 },
    { NULL, 80 }, { NULL, 8 }, { NULL, 32 }
  };
  for( size_t j = 0; j < sizeof( fields ) / sizeof( *fields ); ++j )
  {
    for( int i = 0; i < min<int>( mBasicInfo.EEGChannels(), mChannelInfo.size() ); ++i )
    {
      static const string empty;
      const string& s = fields[j].s ? mChannelInfo[i].*(fields[j].s) : empty;
      for( size_t k = 0; k < min( s.length(), fields[j].l ); ++k )
        oss.put( s[k] );
      for( size_t k = s.length(); k < fields[j].l; ++k )
        oss.put( ' ' );
    }
    for( size_t i = mChannelInfo.size(); i < static_cast<size_t>( mBasicInfo.EEGChannels() ); ++i )
      for( int k = 0; k < 256; ++k )
        oss.put( ' ' );
  }

  string header = oss.str();
  if( header.length() != 256 * ( 1 + mBasicInfo.EEGChannels() ) )
    throw std::logic_error( "Invalid EDF header" );
  
  NscPacketHeader( 'DATA', DataType_InfoBlock, InfoType_EdfHeader, header.length() ).WriteBinary( os );
  os.write( header.data(), header.length() ).flush();
}

void
NeuroSrv::SendASTSetupFile( std::ostream& os )
{
  NscPacketHeader( 'FILE', SetupFile, 0, 0 ).WriteBinary( os );
  os.flush();
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
/*  messageActions[ NscPacketHeader( 'CTRL', ::GeneralControlCode,::RequestForVersion, 0 ) ]
    = &NeuroSrv::SendVersion;*/
  messageActions[ NscPacketHeader( 'CTRL', ::GeneralControlCode,::ClosingUp, 0 ) ]
    = &NeuroSrv::CloseConnection;
  messageActions[ NscPacketHeader( 'CTRL', ::ClientControlCode, ::RequestEDFHeader, 0 ) ]
    = &NeuroSrv::SendEDFHeader;
  messageActions[ NscPacketHeader( 'CTRL', ::ClientControlCode, ::RequestAstFile, 0 ) ]
    = &NeuroSrv::SendASTSetupFile;
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
    sockstream client;
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
#if WIN32
  ::Sleep( inMilliseconds );
#else
  ::timeval sleepDuration = { 0, 1000 * inMilliseconds /* convert to microseconds */ };
  ::select( 0, NULL, NULL, NULL, &sleepDuration );
#endif
}
