////////////////////////////////////////////////////////////////////////////////
//
// File: RDAQueue.cpp
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Jan 3, 2003
//
// Description: A class that encapsulates connection details of the BrainAmp
//              RDA socket interface.
//
////////////////////////////////////////////////////////////////////////////////
#pragma hdrstop
// Make sure Windows.h is _not_ included before the following line
//  (e.g., via vcl.h or PCHIncludes.h) or linking will fail.
#define INITGUID
#include <Windows.h>
#include "RDA/RecorderRDA.h"

#include "RDAQueue.h"

# include "UBCIError.h"
#include <assert>
#include <string>

using namespace std;

RDAQueue::RDAQueue()
: bufferSize( initialBufferSize ),
  receiveBuffer( ( char* )::malloc( bufferSize ) ),
  failstate( ok ),
  socketHandle( NULL ),
  lastMessageType( RDAStop )
{
  connectionInfo.blockNumber = 0;
  connectionInfo.numChannels = 0;
  connectionInfo.samplingInterval = 0;
  connectionInfo.blockDuration = blockDurationGuess;
  connectionInfo.channelResolutions
    = vector<double>( connectionInfo.numChannels, 0 );

  if( receiveBuffer == NULL )
  {
    failstate |= memoryFail;
    bcierr << "Could not allocate receive buffer" << endl;
  }

  ::WSADATA ignored;
  if( ::WSAStartup( 2, &ignored ) )
  {
    failstate |= netinitFail;
    bcierr << "Could not initialize Windows sockets" << endl;
  }
}

RDAQueue::~RDAQueue()
{
  close();
  ::free( receiveBuffer );
  ::WSACleanup();
}

void
RDAQueue::open( const char* inHostName )
{
  if( failstate != ok )
    return;

  if( is_open() )
    close();

  string hostName = inHostName;
  socketHandle = ::socket( PF_INET, SOCK_STREAM, 0 );
  if( socketHandle == SOCKET_ERROR )
  {
    failstate |= netinitFail;
    bcierr << "Could not create client socket" << endl;
    socketHandle = NULL;
    return;
  }

  sockaddr_in address;
  ::memset( &address, 0, sizeof( address ) );
  address.sin_family = AF_INET;
  address.sin_port = ::htons( RDAPortNumber );

  // A connection via "localhost" (127.0.0.1) uses internal buffers on WIN2000,
  // i.e. a sort of burst mode, so we use normal network access via hostname.
  if( hostName == "localhost" )
  {
    ::gethostname( receiveBuffer, bufferSize );
    hostName = receiveBuffer;
  }

  ::hostent* host = ::gethostbyname( hostName.c_str() );
  if( host == NULL )
  {
    failstate |= netinitFail;
    bcierr << "Could not resolve host name" << endl;
    close();
    return;
  }
  if( host->h_addr_list == NULL )
  {
    failstate |= netinitFail;
    bcierr << "Could not resolve host name" << endl;
    close();
    return;
  }
  address.sin_addr = *reinterpret_cast<in_addr*>( host->h_addr_list[ 0 ] );

  if( ::connect( socketHandle, reinterpret_cast<sockaddr*>( &address ), sizeof( address ) )
        == SOCKET_ERROR )
  {
    failstate |= connectionFail;
    bcierr << "Could not connect to " << hostName << endl;
    close();
    return;
  }

  connectionInfo.blockDuration = blockDurationGuess;
  ReceiveData();
}

void
RDAQueue::close()
{
  ::closesocket( socketHandle );
  socketHandle = NULL;
  lastMessageType = RDAStop;
  clear();
}

const short&
RDAQueue::front()
{
  while( empty() && failstate == ok )
    ReceiveData();
  if( failstate == ok )
    return queue<short>::front();
  static short null = 0;
  return null;
}

void
RDAQueue::ReceiveData()
{
  if( lastMessageType == RDAStop )
  {
    // Wait for the start block to appear.
    if( connectionInfo.blockDuration < 1 )
      connectionInfo.blockDuration = 1;
    ::timeval timeout = { 0, connectionInfo.blockDuration };
    ::fd_set readfds;
    FD_ZERO( &readfds );
    FD_SET( socketHandle, &readfds );
    int result = ::select( 1, &readfds, NULL, NULL, &timeout );
    if( result == 0 )
    {
      size_t numEntries
        = ( connectionInfo.blockDuration * connectionInfo.numChannels )
            / connectionInfo.samplingInterval;
      for( size_t i = 0; i < numEntries; ++i )
        push( 0 );
      return;
    }
    else if( result != 1 )
    {
      failstate |= connectionFail;
      bcierr << "Could not read data" << endl;
      return;
    }
  }

  GetServerMessage();
  if( failstate )
  {
    lastMessageType = RDAStop;
    return;
  }
  const RDA_MessageHeader* msg
    = reinterpret_cast<const RDA_MessageHeader*>( receiveBuffer );
  switch( msg->nType )
  {
    case RDAStart:
      {
        const RDA_MessageStart* startMsg
          = reinterpret_cast<const RDA_MessageStart*>( msg );
        connectionInfo.numChannels = startMsg->nChannels;
        connectionInfo.samplingInterval = startMsg->dSamplingInterval;
        connectionInfo.channelResolutions.clear();
        for( size_t i = 0; i < startMsg->nChannels; ++i )
          connectionInfo.channelResolutions.push_back( startMsg->dResolutions[ i ] );
        clear();
      }
      break;
    case RDAStop:
      break;
    case RDAData:
      {
        const RDA_MessageData* dataMsg
          = reinterpret_cast<const RDA_MessageData*>( msg );
        switch( lastMessageType )
        {
          case RDAData:
            if( ( ++connectionInfo.blockNumber & blockNumberMask )
                != ( dataMsg->nBlock & blockNumberMask ) )
            {
              failstate |= connectionFail;
              bcierr << "RDA block numbers not in sequence" << endl;
              lastMessageType = RDAStop;
            }
            break;
          case RDAStart:
            connectionInfo.blockNumber = dataMsg->nBlock;
            connectionInfo.blockDuration = dataMsg->nPoints * connectionInfo.samplingInterval;
            break;
          case RDAStop:
            failstate |= connectionFail;
            bcierr << "RDA server sent data block before start block" << endl;
            return;
          default:
            assert( false ); // Only the named three values should ever be in lastMessageType.
        }

        // Unlike explicitly stated in RecorderRDA's RDA_MessageData declaration,
        // the "Markers[]" array is neither an array nor located where the
        // struct says. What a mess...
        const RDA_Marker* markersBegin =
            ( RDA_Marker* )&dataMsg->nData[ dataMsg->nPoints * connectionInfo.numChannels ];

        size_t dataIndex = 0;
        for( size_t point = 0; point < dataMsg->nPoints; ++point )
        {
          for( size_t channel = 0; channel < connectionInfo.numChannels; ++channel )
            push( dataMsg->nData[ dataIndex++ ] );

          // Construct the marker channel for this data point.
          // Markers are rare, so reading them into more efficient data
          // structures _before_ the sample point loop would not pay out.
          unsigned short markerState = 0;
          // Loop through all markers (ugly data structures, ugly loops, sorry...)
          size_t markerIndex;
          const RDA_Marker* marker;
          for( marker = markersBegin, markerIndex = 0;
               markerIndex < dataMsg->nMarkers;
               ( char* )marker += marker->nSize, ++markerIndex )
            // Does the marker say anything about this point?
            if( point >= marker->nPosition && point < marker->nPosition + marker->nPoints )
            {
              const char* markerDesc = marker->sTypeDesc + ::strlen( marker->sTypeDesc ) + 1;
              switch( *markerDesc )
              {
                case 'S':
                  markerState |= ::atoi( markerDesc + 1 );
                  break;
                case 'R':
                  markerState |= ( ::atoi( markerDesc + 1 ) ) << 8;
                  break;
              }
            }
          // After the data channels, write the marker state channel.
          push( markerState );
        }

      }
      break;
    default:
      ; // Ignoring undocumented message types.
  }
  switch( msg->nType )
  {
    case RDAStart:
    case RDAStop:
    case RDAData:
      lastMessageType = RDAMessageType( msg->nType );
      break;
    default:
      ; // Ignoring undocumented message types.
  }
}

void
RDAQueue::GetServerMessage()
{
  char* pData = receiveBuffer;
  int nRemLength = sizeof( RDA_MessageHeader );
  bool bFirstRecv = true;
  // Retrieve header.
  while( nRemLength > 0 )
  {
    int nResult = ::recv( socketHandle, pData, nRemLength, 0 );

    if( nResult == 0 && bFirstRecv )
    {
      close();
      return;
    }
    bFirstRecv = false;
    if( nResult < 0 )
    {
      failstate |= connectionFail;
      bcierr << "Could not receive data" << endl;
      return;
    }
    nRemLength -= nResult;
    pData += nResult;
  }

  // Check for correct header GUID.
  const RDA_MessageHeader* header = reinterpret_cast<const RDA_MessageHeader*>( receiveBuffer );
  if( header->guid != GUID_RDAHeader )
  {
    failstate |= connectionFail;
    bcierr << "RDA message doesn't have correct GUID" << endl;
    return;
  }
  nRemLength = header->nSize - sizeof( RDA_MessageHeader );
  if( bufferSize < header->nSize )
  {
    bufferSize = header->nSize;
    receiveBuffer = ( char* )::realloc( receiveBuffer, bufferSize );
    if( receiveBuffer == NULL )
    {
      failstate |= memoryFail;
      bcierr << "Could not adapt size of receive buffer" << endl;
      return;
    }
  }
  pData = receiveBuffer + sizeof( RDA_MessageHeader );

  // Retrieve rest of block.
  while( nRemLength > 0 )
  {
    int nResult = ::recv( socketHandle, pData, nRemLength, 0 );
    if( nResult <= 0 )
    {
      failstate |= connectionFail;
      bcierr << "Error receiving data" << endl;
      return;
    }
    nRemLength -= nResult;
    pData += nResult;
  }
}

