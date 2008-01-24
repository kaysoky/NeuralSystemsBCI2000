/******************************************************************************
 * Program:   EEGsource.EXE                                                   *
 * Module:    NeuroscanADC.CPP                                                *
 * Comment:   Definition for the GenericADC class                             *
 * Version:   1.00                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V1.00 - 04/28/2004 - First working version                                 *
 * V1.10 - 05/17/2004 - Now includes support for 32bit data                   *
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "NeuroscanADC.h"

#include "GenericSignal.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

using namespace std;

// Register the source class with the framework.
RegisterFilter( NeuroscanADC, 1 );

// **************************************************************************
// Function:   NeuroscanADC
// Purpose:    The constructor for the NeuroscanADC
//             it fills the provided list of parameters and states
//             with the parameters and states it requests from the operator
// Parameters: plist - the list of parameters
//             slist - the list of states
// Returns:    N/A
// **************************************************************************
NeuroscanADC::NeuroscanADC()
: samplingrate( -1 ),
  server( NULL ),
  c_tcpsocket( NULL)
{
 // add all the parameters that this ADC requests to the parameter list
 BEGIN_PARAMETER_DEFINITIONS
   "Source int SourceCh=      16 16 1 128 "
       "// number of digitized channels (has to match Neuroscan)",
   "Source int SampleBlockSize= 32 5 1 128 "
       "// number of samples per block (has to match Neuroscan)",
   "Source int SamplingRate=    256 128 1 40000 "
       "// the signal sampling rate (has to match Neuroscan)",
   "Source string ServerAddress= localhost:3999 "
       "// address and port of the Neuroscan Acquire server",
 END_PARAMETER_DEFINITIONS

 // add all states that this ADC requests to the list of states
 // this is just an example (here, we don't really need all these states)
 BEGIN_STATE_DEFINITIONS
   "Running 1 0 0 0",
   "SourceTime 16 2347 0 0",
   "NeuroscanEvent1 8 0 0 0",
 END_STATE_DEFINITIONS
}

NeuroscanADC::~NeuroscanADC()
{
 Halt();
 if (server) delete server;
 if (c_tcpsocket) delete c_tcpsocket;
}


// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void NeuroscanADC::Preflight( const SignalProperties&,
                                    SignalProperties& outSignalProperties ) const
{
AcqBasicInfo  m_BasicInfo;
int   num_channels, num_markerchannels, blocksize, samplingrate, bitspersample;
float LSB;

  //
  // Connect to the server and gather basic info to compare against the BCI2000 parameter settings
  //
  client_tcpsocket s( Parameter("ServerAddress").c_str() );
  sockstream server( s );
  if ( !server.is_open() )
     {
     bcierr << "Could not connect to server at the provided address. Make sure Acquire is running and the server is enabled at the correct port." << endl;
     return;
     }

  // send a request for basic information (sampling rate, etc.) to the Acquire server
  const char infoRequest[] = { 'C', 'T', 'R', 'L', 0, (char)ClientControlCode, 0, (char)RequestBasicInfo, 0, 0, 0, 0 };
  server.write( infoRequest, sizeof( infoRequest ) );
  server.flush();

  // read the response from the server
  CAcqMessage *pMsg=new CAcqMessage();
  char *char_ptr=(char *)pMsg;

  // read the header
  for (int i=0; i<NS_HEADER_SIZE; i++)
   {
   server.get( *char_ptr );
   char_ptr++;
   }

  // has the connection closed? then abort
  if ( !server )
     {
     bcierr << "Connection unexpectedly closed by the server " << endl;
     return;
     }

  // Network byte order to host byte order (Big-Endian -> Little-Endian)
  pMsg->Convert(false);

  // Read the body
  int bodyLen = pMsg->m_dwSize;
  if (bodyLen > 0)
     {
     pMsg->m_pBody = new char[bodyLen];
     char_ptr=(char *)pMsg->m_pBody;
     int total = 0;
     while(total < bodyLen)
      {
      server.get( *char_ptr );
      char_ptr++;
      total++;
      }
     }
  else
     pMsg->m_pBody = NULL;

  // Process message
  if (pMsg->IsCtrlPacket())
     {
     bcierr << "Unexpected control packet from the server " << endl;
     return;
     }
   else
     {
      if ((pMsg->IsDataPacket()) && (pMsg->m_wCode == DataType_InfoBlock) && (pMsg->m_wRequest == InfoType_BasicInfo))
         {
         AcqBasicInfo  m_BasicInfo;
         AcqBasicInfo* pInfo = (AcqBasicInfo *)pMsg->m_pBody;
         // let's make sure that the incoming data block is in fact of the correct length
         assert(pInfo->dwSize == sizeof(m_BasicInfo));
         // if it is, copy it into the data structure
         memcpy((void*)&m_BasicInfo, pMsg->m_pBody, sizeof(m_BasicInfo));
         // get the information from the data structure
         num_channels=m_BasicInfo.nEegChan;             // number of data channels
         num_markerchannels=m_BasicInfo.nEvtChan;       // number of event marker channels
         blocksize=m_BasicInfo.nBlockPnts;              // number of samples per block
         samplingrate=m_BasicInfo.nRate;                // sampling rate in Hz
         bitspersample=m_BasicInfo.nDataSize*8;         // bits per sample
         LSB=m_BasicInfo.fResolution;                   // microvolts per LSB
         }
      else
         {
         bcierr << "Unexpected data packet from the server (Code=" << pMsg->m_wCode << ") (Request=" << pMsg->m_wRequest << ")" << endl;
         return;
         }
      }
  delete pMsg;

  const char reqCloseConn[] = { 'C', 'T', 'R', 'L', 0, (char)GeneralControlCode, 0, (char)ClosingUp, 0, 0, 0, 0 };
  server.write( reqCloseConn, sizeof( reqCloseConn ) );
  server.flush();

  // important! we need to give the server a little time to close
  Sleep(25);
  server.close();

  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  // cross check the parameters as received from the server with the BCI2000 parameters
  PreflightCondition( Parameter( "SourceCh" ) == num_channels );
  PreflightCondition( Parameter( "SampleBlockSize" ) == blocksize );
  PreflightCondition( Parameter( "SamplingRate" ) == samplingrate );

  // also cross check SourceChGain and SourceChOffset
  for (int ch=0; ch<Parameter( "SourceCh" ); ch++)
   {
   PreflightCondition( Parameter( "SourceChOffset" )( ch ) == 0 );        // we have to assume that the signal is already calibrated
   PreflightCondition( fabs(Parameter( "SourceChGain" )( ch )-LSB)<(1E-3) );
   }

  // Resource availability checks.
  /* The random source does not depend on external resources. */

  // Input signal checks.
  /* The input signal will be ignored. */

  // Requested output signal properties.
  SignalType outSignalType;
  switch( bitspersample )
  {
    case 16:
      outSignalType = SignalType::int16;
      break;
    case 32:
      outSignalType = SignalType::int32;
      break;
    default:
      bcierr << "Server reports unsupported data size "
             << "(" << bitspersample << " bits per sample)"
             << endl;
  }
  outSignalProperties = SignalProperties(
       Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), outSignalType );
}


void NeuroscanADC::SendCommand(unsigned short controlcode, unsigned short command)
{
  // sanity checks
  if ( !server )
     {
     cerr << "Server object not created " << endl;
     return;
     }
  if ( !server->is_open() )
     {
     cerr << "Connection to Acquire server not established " << endl;
     return;
     }


  // construct the command to be sent out
  char cmd2send[12];
  strcpy(cmd2send, "CTRL");
  unsigned short *cc=(unsigned short *)&cmd2send[4];
  *cc=htons(controlcode);
  unsigned short *cmd=(unsigned short *)&cmd2send[6];
  *cmd=htons(command);
  cmd2send[8]=0;
  cmd2send[9]=0;
  cmd2send[10]=0;
  cmd2send[11]=0;
  // and actually send it out
  server->write( cmd2send, sizeof( cmd2send ) );
  server->flush();
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the NeuroscanADC
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop (fMain->MainDataAcqLoop())
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void NeuroscanADC::Initialize( const SignalProperties&, const SignalProperties& )
{
  // store the value of the needed parameters
  samplingrate = -1;

  //
  // Connect to the server and tell it to start acquiring data
  //
  // just in case that the connection had been established before, shut it down here
  if (server)
     if (server->is_open())
        Halt();
  if (server)      delete server;
  if (c_tcpsocket) delete c_tcpsocket;
  // make the connection
  c_tcpsocket=new client_tcpsocket( Parameter("ServerAddress").c_str() );
  c_tcpsocket->set_tcpnodelay( true );
  server=new sockstream( *c_tcpsocket );
  if ( !server )
     {
     bcierr << "Could not create tcpstream object." << endl;
     return;
     }
  if ( !server->is_open() )
     {
     bcierr << "Could not connect to server at the provided address. Make sure Acquire is running and the server is enabled at the correct port." << endl;
     return;
     }
  // send a request to again get basic info to the Acquire Server (already did that in Preflight())
  SendCommand(ClientControlCode, RequestBasicInfo);
  // send a request to start data acquisition to the Acquire Server
  SendCommand(ServerControlCode, StartAcquisition);
  // send a request to start sending data to the Acquire Server
  SendCommand(ClientControlCode, RequestStartData);
}


// **************************************************************************
// Function:   Process
// Purpose:    This function is called within fMain->MainDataAcqLoop()
//             it fills the already initialized array RawEEG with values
//             and DOES NOT RETURN, UNTIL ALL DATA IS ACQUIRED
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void NeuroscanADC::Process( const GenericSignal&, GenericSignal& signal )
{
 // sanity checks
 if ( !server )
    {
    bcierr << "No connection created in Process() ??" << endl;
    return;
    }
 if ( !server->is_open() )
    {
    bcierr << "No connection to the Acquire server in Process() ?" << endl;
    return;
    }

  //
  // let's receive one data packet from the Acquire server
  //
  bool finished=false;
  while (!finished)     // do this until we receive an EEG data packet
   {
   CAcqMessage *pMsg=new CAcqMessage();
   char *char_ptr=(char *)pMsg;

   // read the header
   for (int i=0; i<NS_HEADER_SIZE; i++)
    {
    server->get( *char_ptr );
    char_ptr++;
    }

   // has the connection closed? then abort
   if ( !*server ) break;

   // Network byte order to host byte order (Big-Endian -> Little-Endian)
   pMsg->Convert(false);

   // Read the body
   int bodyLen = pMsg->m_dwSize;
   if (bodyLen > 0)
      {
      pMsg->m_pBody = new char[bodyLen];
      char_ptr=(char *)pMsg->m_pBody;
      // read bodyLen-1 characters using read and the last character using get
      // supposed to do this when using STL-streams
      server->read( char_ptr, bodyLen-1 );
      char_ptr += bodyLen-1;
      server->get( *char_ptr );
      }
   else
      pMsg->m_pBody = NULL;

   // Process message
   if (pMsg->IsCtrlPacket())
      {
      // printf("Receiving Control Message\n");
      // ProcessCtrlMsg(pMsg); pMsg=NULL;
      }
   else
      if (pMsg->IsDataPacket())    // looks like we get one packet every 40 ms (25/sec)
         finished=ProcessDataMsg(pMsg, &signal);

   delete pMsg;
   }
}


//////////////////////////////////////////////////////////////////////
// Process "DATA" packet
// returns true if we got an EEG data packet, otherwise false
//////////////////////////////////////////////////////////////////////
bool NeuroscanADC::ProcessDataMsg(CAcqMessage *pMsg, GenericSignal *signal)
{
bool          retval;

 retval=false;

 if (pMsg->m_wCode == DataType_InfoBlock)
    {
    if (pMsg->m_wRequest == InfoType_BasicInfo)
       {
       AcqBasicInfo  m_BasicInfo;
       AcqBasicInfo* pInfo = (AcqBasicInfo *)pMsg->m_pBody;
       // let's make sure that the incoming data block is in fact of the correct length
       assert(pInfo->dwSize == sizeof(m_BasicInfo));
       // if it is, copy it into the data structure
       memcpy((void*)&m_BasicInfo, pMsg->m_pBody, sizeof(m_BasicInfo));
       // get the information from the data structure
       num_channels=m_BasicInfo.nEegChan;             // number of data channels
       num_markerchannels=m_BasicInfo.nEvtChan;       // number of event marker channels
       blocksize=m_BasicInfo.nBlockPnts;              // number of samples per block
       samplingrate=m_BasicInfo.nRate;                // sampling rate in Hz
       bitspersample=m_BasicInfo.nDataSize*8;         // bits per sample
       LSB=m_BasicInfo.fResolution;                   // microvolts per LSB
       // we have cross-checked these values in Preflight() before,
       // thus, do not need to cross-check again
       // now, do remember these values for later
       }
    }
 else if (pMsg->m_wCode == DataType_EegData)
    {
      if (pMsg->m_dwSize != size_t((num_channels+num_markerchannels)*blocksize*(bitspersample/8)))
        bcierr << "Inconsistent data message block size" << endl;
      if( samplingrate <= 0 )
        bcierr << "Received data block before info block" << endl;

      switch( pMsg->m_wRequest )
      {
        case DataTypeRaw16bit:
        case DataTypeRaw32bit:
          {
            const unsigned char* pData = reinterpret_cast<unsigned char*>( pMsg->m_pBody );
            // we also write the event marker channel into a state variable
            // in the current implementation of BCI2000, we can only have one event marker
            // per sample block, not per sample
            // let's use the event marker of the first sample for the whole sample block
            if( num_markerchannels > 0 )
              State( "NeuroscanEvent1" ) = pData[ num_channels * ( bitspersample / 8 ) ]; // mask out lower 8 bits

            for( int sample = 0; sample < blocksize; ++sample )
            {
              for( int channel = 0; channel < num_channels; ++channel )
              {
                signed long value = 0;
                for( int byte = 0; byte < bitspersample / 8 - 1; ++byte )
                  value |= ( *pData++ ) << ( 8 * byte );
                // for little-endian data, the last byte determines the sign
                signed char signedByte = *pData++;
                value |= signedByte << ( bitspersample - 8 );
                ( *signal )( channel, sample ) = value;
              }
              pData += num_markerchannels * ( bitspersample / 8 );
            }
          }
          break;
        default:
           bcierr << "Unrecognized EEG data type." << endl;
      }
      retval = true;
    }

 return(retval);
}


// **************************************************************************
// Function:   Halt
// Purpose:    This routine shuts down data acquisition
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void NeuroscanADC::Halt()
{
 // sanity checks
 if (!server) return;
 if (!server->is_open()) return;

 // send the command to stop sending data
 SendCommand(ClientControlCode, RequestStopData);

 // send the command to close the connection
 SendCommand(GeneralControlCode, ClosingUp);

 // important! we need to give the server a little time to close
 Sleep(25);

 // actually close down the connection
 server->close();
}



