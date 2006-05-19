/******************************************************************************
 * Program:   EEGsource.EXE                                                   *
 * Module:    MicromedADC.CPP                                                 *
 * Comment:   Definition for the GenericADC class                             *
 * Version:   0.02                                                            *
 * Author:    Gerwin Schalk, Juergen Mellinger, Erik Aarnoutse                *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 19/04/2006 - First working version, based on Neusoscan.exe         *                       *
 * V0.02 - 08/05/2006 - Added condition testing, 22bit mode and downsampling  *
 * V0.03 - 17/05/2006 - Added notchfilter                                     *
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "MicromedADC.h"
#include "UBCIError.h"
#include "UGenericSignal.h"
#include <stdio.h>
#include <math.h>

#ifndef _W32
  #define _W32
#endif
#ifndef _WIN32
  #define _WIN32
#endif
#ifndef __BORLANDC__
  #define __BORLANDC__
#endif

using namespace std;

// Register the source class with the framework.
RegisterFilter( MicromedADC, 1 );

// **************************************************************************
// Function:   MicromedADC
// Purpose:    The constructor for the MicromedADC
//             it fills the provided list of parameters and states
//             with the parameters and states it requests from the operator
// Parameters: plist - the list of parameters
//             slist - the list of states
// Returns:    N/A
// **************************************************************************
MicromedADC::MicromedADC()
: samplerate( 9 ),
  mSignalType( SignalType::int16 ),
  MMblocksize( 9 ),
  BCIblocksize( 9 )
  
{
 // add all the parameters that this ADC requests to the parameter list
 BEGIN_PARAMETER_DEFINITIONS
   "Source string ServerAddress= *:5000 1 1 1"
       "// address and port of the Micromed BCI Server",
   "Source int SoftwareCh=      128 16 1 128 "
       "// the number of digitized and stored channels",
   "Source int SampleBlockSize= 8 5 1 128 "
       "// the number of samples transmitted at a time",
   "Source int SamplingRate=    512 128 1 4000 "
       "// the sample rate",
   "Source int SignalType=           0 0 0 3"
        "// numeric type of output signal: "
            " 0: int16,"
            " 1: float24,"
            " 2: float32,"
            " 3: int32 "
            "(enumeration)",
 //"Source string MultiplierState=   -1 -1 0 0 "
 //    "// State to use as signal multiplier (-1 == don't use multiplier)",
 END_PARAMETER_DEFINITIONS

 // add all states that this ADC requests to the list of states
 // this is just an example (here, we don't really need all these states)
 BEGIN_STATE_DEFINITIONS
   "Running 1 0 0 0",
   "SourceTime 16 2347 0 0",
   "MicromedEvent 8 0 0 0"
 END_STATE_DEFINITIONS
}

MicromedADC::~MicromedADC()
{
Halt();
}


// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void MicromedADC::Preflight( const SignalProperties&,
                                       SignalProperties& outSignalProperties ) const
{
  int signalType = Parameter( "SignalType" );

  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  PreflightCondition( Parameter( "SamplingRate" ) % MICROMED_PACKET_RATE==0 );
  PreflightCondition( Parameter( "SampleBlockSize" ) % (Parameter( "SamplingRate" ) / MICROMED_PACKET_RATE)==0);
  PreflightCondition( Parameter( "SignalType" )== 0 || Parameter( "SignalType" )== 3);
  // Resource availability checks.

  // Input signal checks.

  // Requested output signal properties.
  signalType=Parameter( "SignalType" );
  outSignalProperties = SignalProperties(
  Parameter( "SoftwareCh" ),Parameter( "SampleBlocksize" ),SignalType::Type( signalType ));



  }

// **************************************************************************
// Function:   ADInit
// Purpose:    This function parameterizes the MicromedADC
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop (fMain->MainDataAcqLoop())
// Parameters: N/A
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void MicromedADC::Initialize()
{
  // store the value of the needed parameters
  samplerate = Parameter( "SamplingRate" );
  mSignalType = Parameter( "SignalType" );
  MMblocksize=Parameter( "SamplingRate" ) / MICROMED_PACKET_RATE;
  BCIblocksize=Parameter( "SampleBlocksize" );

  // Micromed systemPLUS is the client, BCI is the server, it passively listens.

  // start the server

  MmServerSocket.open(Parameter("ServerAddress"));

  //bciout << "Starting Micromed BCI Server" << endl;

  if( !MmServerSocket.is_open())
    bcierr << "Could not open Micromed BCI Server socket for listening" << endl;
  else {
    bciout << "Micromed BCI Server listening on port "
    << MmServerSocket.port()
    << endl;
    }

  MmServer.close();
  MmServer.clear();

  if( MmServerSocket.is_open() && !MmServerSocket.connected() )
    {
    if (MmServerSocket.wait_for_read( cMmConnectionTimeout, true )) {
      bciout << "Connected with "
      << MmServerSocket.ip()
      << ":"
      << MmServerSocket.port()
      << endl;
    }
    else{
      bcierr << "Connection failed" << endl;
      return;
      }
     }

  MmServer.open( MmServerSocket );
  if( !MmServer.is_open() )
  {
    bciout << "\nConnection to Micromed client timed out after "
           << float( cMmConnectionTimeout ) / 1e3 << "s"
           << endl;
    MmServer.close();
    MmServerSocket.close();
    return;
  }
  if( MmServerSocket.connected() )
    bciout << "Accepting connection with client" << endl;

 }


// **************************************************************************
// Function:   ADReadDataBlock
// Purpose:    This function is called within fMain->MainDataAcqLoop()
//             it fills the already initialized array RawEEG with values
//             and DOES NOT RETURN, UNTIL ALL DATA IS ACQUIRED
// Parameters: N/A
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void MicromedADC::Process( const GenericSignal*, GenericSignal* signal )
{

  bool finished=false;
  int bodyLen;
  int nrloops;
  int i;
  int packet=0;

  State( "MicromedEvent" ) = 0;
  packet=0;

  nrloops=BCIblocksize / MMblocksize; //downsampling, because Micromed send 64 packets per second;
  while (packet<nrloops)
    {
    finished=false;
    while (!finished)     // do this until we receive an EEG data packet
      {
      CAcqMessage *pMsg=new CAcqMessage();
      char *char_ptr=(char *)pMsg;

      // read the header

      for (i=0; i<MICROMED_HEADER_SIZE; i++)
        {
        MmServer.get( *char_ptr );
        char_ptr++;
        }
      // has the connection closed? then abort
      if ( !MmServer )
        {
        bcierr << "Connection closed by the client " << endl;
        pMsg->m_dwSize=0;
        finished=true;
       Halt();
       return;
       }

     if (strncmp(pMsg->m_chId,"MICM",4)!=0)    // "MICM" is Micromed
       {
       bcierr << "Sequence error" << endl;
       Halt();
       return;
       }



     bodyLen = pMsg->m_dwSize;

     if (bodyLen > 0)
       {
       pMsg->m_pBody = new char[bodyLen];
       char_ptr=(char *)pMsg->m_pBody;
       int total = 0;

       while(total < bodyLen)
         {
         MmServer.get( *char_ptr );
         char_ptr++;
         total++;
         }
       finished=ProcessDataMsg(pMsg, packet, signal);
       }
     else
       pMsg->m_pBody = NULL;



     delete pMsg;
     }
     packet++;
    }
}

//////////////////////////////////////////////////////////////////////
// Process "DATA" packet
// returns true if we got an EEG data packet, otherwise false
//////////////////////////////////////////////////////////////////////
bool MicromedADC::ProcessDataMsg(CAcqMessage *pMsg, int packetnr, GenericSignal *signal)
{
bool  retval;
SignalProperties outSignalProperties;


 retval=false;
 const unsigned char* pData = reinterpret_cast<unsigned char*>( pMsg->m_pBody );

 if (pMsg->IsHeaderPacket()==0)
      {
            SignalType outSignalType;
            //skip first 141 bytes (patient name, date, etc.)
            for (int byte=0;byte<142;byte++) {*pData++;}

            //read num_channels (int16)
            for( int byte = 0; byte < 2; ++byte )
                {
                  long byteVal = *pData++;
                  num_channels |= byteVal << ( 8 * byte );
                }
            if (num_channels!=Parameter("SoftwareCh"))
            bcierr << "wrong nr of channels in Header" << endl;
            //skip Multiplexer
            for (int byte=0;byte<2;byte++) {*pData++;}
            //read samplerate (int16)
            for( int byte = 0; byte < 2; ++byte )
                {
                  long byteVal = *pData++;
                  samplerate |= byteVal << ( 8 * byte );
                }
            if (samplerate!=Parameter("SamplingRate"))
            bcierr << "wrong sample rate in Header" << endl;

            //read bytespersample (int16)
            for( int byte = 0; byte < 2; ++byte )
                {
                  long byteVal = *pData++;
                  bytespersample |= byteVal << ( 8 * byte );
                }
            switch(bytespersample)
            {
              case 2:
                if (Parameter("SignalType")!=0)
                  bcierr << "signalType in Header is not int16" << endl;
                break;
              case 4:
                if (Parameter("SignalType")!=3)
                  bcierr << "signalType in Header is not int32" << endl;
                break;
              default:
                  bcierr << "unknown signalType in Header" << endl;
                break;
            }

            retval=false; //so BCI reads in a datapacket as well

      }
    else if (pMsg->IsNotePacket()==0)
         {
          State( "MicromedEvent" ) = 1; //Micromed does not send digital triggers in a packet,
                                        //but we can switch on the Analog TriggerIn1, so it sends a note.
                                        //This means that we can't read any serial code.
                                        //Not sure about exact timing.
          retval=false; //so BCI reads in a datapacket as well

          }
      else if (pMsg->IsDataPacket()==0)
          {

		  if (pMsg->m_dwSize != size_t(num_channels*MMblocksize*bytespersample))
            bcierr << "Inconsistent data message block size "
            << num_channels
            << " "
            << MMblocksize
            << " "
            << bytespersample
            << endl;


          signed short int sint16;
          signed long int sint32;

          for( int sample = 0; sample < MMblocksize; ++sample )
          {
            for( int channel = 0; channel < num_channels; ++channel )
            {
              long value = 0;

                for( int byte = 0; byte < bytespersample; ++byte )
                {
                  long byteVal = *pData++;
                  value |= byteVal << ( 8 * byte );
                }
                if (bytespersample==2)
                {
                  value=value-32768; //Micromed sends unsigned short integers
                  sint16=value;
                  ( *signal )( channel, MMblocksize*packetnr+sample ) = sint16;
                }
                else
                {
                 value=value-2097152; //Micromed sends unsigned long integers with 22bit data
                  sint32=value;
                  ( *signal )( channel, MMblocksize*packetnr+sample ) = sint32;
                }
            }
          }
          retval = true;
          }
        else
          {
            bciout << "Unknown Packet" << endl;
            //break;
          }
 return(retval);

}
// **************************************************************************
// Function:   ADShutdown
// Purpose:    This routine shuts down data acquisition
//
// Parameters: N/A
// Returns:    1 ... always
// **************************************************************************
void MicromedADC::Halt()
{
  MmServer.close();
  MmServerSocket.close();
}



