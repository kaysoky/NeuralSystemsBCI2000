#include <iostream>
#include "../shared/TCPStream.h"

#include <stdio.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <fstream> 

#include "UParameter.h"
#include "NeuroscanNetRead.h"

using namespace std;

AcqBasicInfo  m_BasicInfo;

bool ProcessDataMsg(CAcqMessage *pMsg);

template<typename T> string str( T t )
{
  ostringstream oss;
  oss << t;
  return oss.str();
}

int
main( int argc, char** argv )
{
char *address=NULL, *paramfile=NULL;
bool showusage=false;

  printf("BCI2000 Parameter Tool for Neuroscan Acquire V4.3\r\n");
  printf("******************************************************************************\r\n");
  printf("(C)2004 Gerwin Schalk and Juergen Mellinger\r\n");
  printf("        Wadsworth Center, New York State Department of Health, Albany, NY, USA\r\n");
  printf("        Eberhardt-Karls University of Tuebingen, Germany\r\n");
  printf("******************************************************************************\r\n");

  //
  // parameter checks
  //
  // No command line parameter? Then show how to use it
  if (argc == 1) showusage=true;
  // If we have a command line parameter, we need to have an even number
  if ((argc%2 == 0) && (argc > 1))
     {
     printf("\r\nParameter argument missing\r\n\r\n");
     showusage=true;
     }
  // cross-check the parameters against known parameters
  if (argc > 1)
     {
     for (int i=1; i<argc; i+=2)
      {
      if ((strcmp("-address", argv[i]) != 0) && (strcmp("-paramfile", argv[i]) != 0))
         {
         printf("\r\nIllegal parameter %s\r\n\r\n", argv[i]);
         showusage=true;
         }
      if ((strcmp("-address", argv[i]) == 0) && (argc > i+1))
         address=argv[i+1];
      if ((strcmp("-paramfile", argv[i]) == 0) && (argc > i+1))
         paramfile=argv[i+1];
      }
     }

  // show usage if necessary
  if (showusage)
     {
     printf("Usage: neuroscangetparams [-address IP/port] [-paramfile filename]\r\n");
     printf("       e.g., neuroscangetparams -address localhost:3999 -paramfile test.prm\r\n");
     return(0);
     }

  // open up the connection to the server
  client_tcpsocket s( address );
  tcpstream server( s );
  if( !server.is_open() )
  {
    cerr << "Could not connect to " << address << endl;
    return -1;
  }

  // query the server for some basic information
  const char versionRequest[] = { 'C', 'T', 'R', 'L', 0, (char)ClientControlCode, 0, (char)RequestBasicInfo, 0, 0, 0, 0 };
  server.write( versionRequest, sizeof( versionRequest ) );
  server.flush();

  // sanity check
  if( !server )
  {
    cerr << "Error sending request" << endl;
    return -1;
  }

  // eat server messages until we received the info we were looking for
  char c;
  bool finished=false;
  while (!finished)
   {
   CAcqMessage *pMsg=new CAcqMessage();
   char *char_ptr=(char *)pMsg;

   // read the header
   for (int i=0; i<NS_HEADER_SIZE; i++)
    {
    server.get( *char_ptr );
    char_ptr++;
    }

   // has the connection closed? then abort
   if ( !server ) break;

   // Network byte order to host byte order (Big-Endian -> Little-Endian)
   pMsg->Convert(false);

   // Read the body
   int bodyLen = pMsg->m_dwSize;
   // printf("Code: %d Request: %d Body length: %d\n", pMsg->m_wCode, pMsg->m_wRequest, bodyLen);
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
      printf("Receiving Control Message\n");
      // ProcessCtrlMsg(pMsg); pMsg=NULL;
      }
   else
      if (pMsg->IsDataPacket())    // looks like we get one packet every 40 ms (25/sec)
         {
         // printf("Receiving Data Message\n");
         finished=ProcessDataMsg(pMsg);  // if we received a data packet with the correct info, terminate the loop
         }

   delete pMsg;
   }

  // prepare the server for closing the connection
  const char reqCloseConn[] = { 'C', 'T', 'R', 'L', 0, (char)GeneralControlCode, 0, (char)ClosingUp, 0, 0, 0, 0 };
  server.write( reqCloseConn, sizeof( reqCloseConn ) );
  server.flush();

  // important! we need to give the server a little time to close
  Sleep(50);

  // actually close the connection
  server.close();

  // now, transfer these values into parameters to be used by BCI2000 (if desired)
  if (!paramfile) return(0);

  // create BCI2000 parameter objects
  PARAMLIST paramlist;
  const char* params[] =
  {
    "Source int SampleBlockSize= ",
    "Source int SamplingRate= ",
    "Source string ServerAddress= ",
    "Source int SoftwareCh= ",
    "Filtering int AlignChannels= ",
    "Filtering floatlist SourceChGain= 0 ",
    "Filtering floatlist SourceChOffset= 0 ",
    "Filtering floatlist SourceChTimeOffset= 0 ",
  };

  // create a parameter list from these individual parameters
  for( size_t i = 0; i < sizeof( params ) / sizeof( *params ); ++i )
    paramlist.AddParameter2List( ( string( params[ i ] ) + " // neurogetparams " + string(address)).c_str() );

  // set the values to the ones received from the server
  paramlist[ "SampleBlockSize" ].SetValue( str(m_BasicInfo.nBlockPnts) );
  paramlist[ "SamplingRate" ].SetValue( str(m_BasicInfo.nRate) );
  paramlist[ "ServerAddress" ].SetValue( address );
  paramlist[ "SoftwareCh" ].SetValue( str(m_BasicInfo.nEegChan) );
  paramlist[ "AlignChannels" ].SetValue( str(-1) );
  paramlist[ "SourceChGain" ].SetNumValues( m_BasicInfo.nEegChan );
  paramlist[ "SourceChOffset" ].SetNumValues( m_BasicInfo.nEegChan );
  paramlist[ "SourceChTimeOffset" ].SetNumValues( 1 );
  paramlist[ "SourceChTimeOffset" ].SetValue( str(-1) );             // default behavior; do not know how Neuroscan samples

  // set offset for all channels to 0 and gain to the value for LSB as communicated from the server
  for( int i = 0; i < m_BasicInfo.nEegChan; ++i )
  {
    paramlist[ "SourceChOffset" ].SetValue( "0", i );
    paramlist[ "SourceChGain" ].SetValue( str(m_BasicInfo.fResolution), i );
  }

  // open a file stream to output the parameter file
  ofstream fs(paramfile);
  if (!fs.is_open())
     printf("Error opening output parameter file %s\n", paramfile);
  else
     {
     fs << paramlist;  // and write the parameter list to the output file
     printf("Parameter file %s successfully written\n", paramfile);
     }

 return 0;
}


//////////////////////////////////////////////////////////////////////
// Process "DATA" packet
//////////////////////////////////////////////////////////////////////
bool ProcessDataMsg(CAcqMessage *pMsg)
{
 if (pMsg->m_wCode == DataType_InfoBlock)
    {
    if (pMsg->m_wRequest == InfoType_BasicInfo)
       {
       // printf("Processing info data block\n");
       AcqBasicInfo* pInfo = (AcqBasicInfo *)pMsg->m_pBody;
       // let's make sure that the incoming data block is in fact of the correct length
       assert(pInfo->dwSize == sizeof(m_BasicInfo));
       // if it is, copy it into the data structure
       memcpy((void*)&m_BasicInfo, pMsg->m_pBody, sizeof(m_BasicInfo));
       printf("Signal Channels: %d\n"
              "Event Channels:  %d\n"
              "Block Size:      %d\n"
              "Sampling Rate:   %d\n"
              "Bits/Sample:     %d\n"
              "Resolution:      %.3fuV/LSB\n",
       				m_BasicInfo.nEegChan,m_BasicInfo.nEvtChan,m_BasicInfo.nBlockPnts,
				m_BasicInfo.nRate,m_BasicInfo.nDataSize*8,m_BasicInfo.fResolution);
       return true;
       }
    }
 else if (pMsg->m_wCode == DataType_EegData)
    {
    if (pMsg->m_wRequest == DataTypeRaw16bit)
       {
       // printf("Processing 16 bit raw data block\n");
       // make sure the body has the right length - even if we're not processing the data
       assert((int)(pMsg->m_dwSize) == (int)(m_BasicInfo.nEegChan+m_BasicInfo.nEvtChan)*m_BasicInfo.nBlockPnts*m_BasicInfo.nDataSize);
       // process raw 16 bit data
       /* FILE *fp=fopen("eeg.dat", "ab");
       for (int samp=0; samp<m_BasicInfo.nBlockPnts; samp++)
        {
        for (int ch=0; ch<m_BasicInfo.nEegChan+m_BasicInfo.nEvtChan; ch++)
         {
         short *sample=(short *)pMsg->m_pBody;
         // fprintf(fp, "%d ", sample[samp*(m_BasicInfo.nEegChan+m_BasicInfo.nEvtChan)+ch]);
         }
        // fprintf(fp, "\r\n");
        }
       fclose(fp); */
       }
    }

 return false;
}


