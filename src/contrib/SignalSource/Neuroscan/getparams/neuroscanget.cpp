/* $BEGIN_BCI2000_LICENSE$
 * 
 * This file is part of BCI2000, a platform for real-time bio-signal research.
 * [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
 * 
 * BCI2000 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * BCI2000 is distributed in the hope that it will be useful, but
 *                         WITHOUT ANY WARRANTY
 * - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * $END_BCI2000_LICENSE$
 */
#include "SockStream.h"

#include "BCIAssert.h"
#include "ParamList.h"
#include "NeuroscanNetRead.h"

#include <iostream>

#include <stdio.h>
#include <string>
#include <sstream>
#include <fstream>


using namespace std;

AcqBasicInfo  m_BasicInfo;
vector<string> gChannelNames;
bool gSilent = false;
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
bool extendedInfo = false,
     toStdout = false;
bool showusage=(argc < 2);

  //
  // parameter checks
  //
  int i = 1;
  while( i < argc )
  {
    if( !strcmp( "-address", argv[i] ) )
    {
      if( ++i < argc )
        address = argv[i];
      else
        showusage = true;
    }
    else if( !strcmp( "-paramfile", argv[i] ) )
    {
      if( ++i < argc )
        paramfile = argv[i];
      else
        showusage = true;
    }
    else if( !strcmp( "-e", argv[i] ) )
      extendedInfo = true;
    else if( !address && *argv[i] != '-' )
    {
      address = argv[i];
      extendedInfo = true;
      toStdout = true;
      gSilent = true;
    }
    else
      showusage = true;
    ++i;
  }
    
  if( !gSilent )
  {
    printf("BCI2000 Parameter Tool for Neuroscan Acquire V4.3\r\n");
    printf("******************************************************************************\r\n");
    printf("(C)2004 Gerwin Schalk and Juergen Mellinger\r\n");
    printf("        Wadsworth Center, New York State Department of Health, Albany, NY, USA\r\n");
    printf("        Eberhard-Karls University of Tuebingen, Germany\r\n");
    printf("******************************************************************************\r\n");
  }
  // show usage if necessary
  if (showusage)
     {
     printf("Usage: neuroscangetparams [-address IP/port] [-paramfile filename] [-e]\r\n");
     printf("       e.g., neuroscangetparams -address localhost:3999 -paramfile test.prm\r\n");
     printf("       use -e to obtain extended information such as channel names\r\n");
     return(0);
     }

  // open up the connection to the server
  client_tcpsocket s( address );
  sockstream server( s );
  if( !server.is_open() )
  {
    cerr << "Could not connect to " << address << endl;
    return -1;
  }

  int messagesExpected = 0;

  // query the server for some basic information
  const char versionRequest[] = { 'C', 'T', 'R', 'L', 0, (char)ClientControlCode, 0, (char)RequestBasicInfo, 0, 0, 0, 0 };
  server.write( versionRequest, sizeof( versionRequest ) );
  server.flush();
  ++messagesExpected;

  if( extendedInfo )
  {
    const char edfRequest[] = { 'C', 'T', 'R', 'L', 0, (char)ClientControlCode, 0, (char)RequestEdfHeader, 0, 0, 0, 0 };
    server.write( edfRequest, sizeof( edfRequest ) );
    server.flush();
    ++messagesExpected;
  }
  
  // sanity check
  if( !server )
  {
    cerr << "Error sending request" << endl;
    return -1;
  }

  // eat server messages until we received the info we were looking for
  while (messagesExpected)
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
         ProcessDataMsg(pMsg);  // if we received a data packet with the correct info, terminate the loop
         }

   delete pMsg;
   --messagesExpected;
   }

  // prepare the server for closing the connection
  const char reqCloseConn[] = { 'C', 'T', 'R', 'L', 0, (char)GeneralControlCode, 0, (char)ClosingUp, 0, 0, 0, 0 };
  server.write( reqCloseConn, sizeof( reqCloseConn ) );
  server.flush();
 
  // actually close the connection
  s.close();

  // create BCI2000 parameter objects
  ParamList paramlist;
  const char* params[] =
  {
    "Source int SampleBlockSize= ",
    "Source int SamplingRate= ",
    "Source string ServerAddress= ",
    "Source int SourceCh= ",
    "Filtering int AlignChannels= ",
    "Filtering floatlist SourceChGain= 0 ",
    "Filtering floatlist SourceChOffset= 0 ",
  };

  // create a parameter list from these individual parameters
  for( size_t i = 0; i < sizeof( params ) / sizeof( *params ); ++i )
    paramlist.Add( ( string( params[ i ] ) + " % // neurogetparams " + string(address)).c_str() );

  // set the values to the ones received from the server
  paramlist[ "SampleBlockSize" ].Value() = str(m_BasicInfo.nBlockPnts);
  paramlist[ "SamplingRate" ].Value() = str(m_BasicInfo.nRate);
  paramlist[ "ServerAddress" ].Value() = address;
  paramlist[ "SourceCh" ].Value() = str(m_BasicInfo.nEegChan);
  paramlist[ "AlignChannels" ].Value() = "0";
  paramlist[ "SourceChGain" ].SetNumValues( m_BasicInfo.nEegChan );
  paramlist[ "SourceChOffset" ].SetNumValues( m_BasicInfo.nEegChan );

  // set offset for all channels to 0 and gain to the value for LSB as communicated from the server
  for( int i = 0; i < m_BasicInfo.nEegChan; ++i )
  {
    paramlist[ "SourceChOffset" ].Value( i ) = "0";
    paramlist[ "SourceChGain" ].Value( i ) = str(m_BasicInfo.fResolution);
  }
  
  if( !gChannelNames.empty() )
  {
    paramlist.Add( "Source stringlist ChannelNames= 0 // neurogetparams " + string(address) );
    paramlist["ChannelNames"].SetNumValues( gChannelNames.size() );
    for( size_t i = 0; i < gChannelNames.size(); ++i )
      paramlist["ChannelNames"].Value( i ) = gChannelNames[i];
  }

  // open a file stream to output the parameter file
  if( paramfile )
  {
    ofstream fs(paramfile);
    if (!fs.is_open())
       fprintf(stderr,"Error opening output parameter file %s\n", paramfile);
    else if( fs << paramlist && !gSilent )  // and write the parameter list to the output file
       printf("Parameter file %s successfully written\n", paramfile);
  }
  if( toStdout )
    cout << paramlist;

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
       bciassert(pInfo->dwSize == sizeof(m_BasicInfo));
       // if it is, copy it into the data structure
       memcpy((void*)&m_BasicInfo, pMsg->m_pBody, sizeof(m_BasicInfo));
       if( !gSilent )
       {
         printf("Signal Channels: %d\n"
                "Event Channels:  %d\n"
                "Block Size:      %d\n"
                "Sampling Rate:   %d\n"
                "Bits/Sample:     %d\n"
                "Resolution:      %.3fuV/LSB\n",
                m_BasicInfo.nEegChan,m_BasicInfo.nEvtChan,m_BasicInfo.nBlockPnts,
                m_BasicInfo.nRate,m_BasicInfo.nDataSize*8,m_BasicInfo.fResolution);
       }
       return true;
       }
     else if( pMsg->m_wRequest == InfoType_EdfHeader )
     {
       gChannelNames.clear();
       string edfHeader( pMsg->m_pBody, pMsg->m_dwSize );
       if( edfHeader.find( "0       " ) == 0 )
       {
         int channels;
         const int channelBegin = 256;
         if( istringstream( edfHeader.substr( channelBegin - 4, 4 ) ) >> channels )
         {
           for( int i = 0; i < channels; ++i )
           {
             string channelName = edfHeader.substr( channelBegin + 16 * i, 16 );
             string suppress[] = { "EEG", "MEG", };
             for( size_t i = 0; i < sizeof( suppress ) / sizeof( *suppress ); ++i )
             {
               size_t pos = channelName.find( suppress[i] );
               if( pos != string::npos )
                 channelName = channelName.substr( 0, pos ) + channelName.substr( pos + suppress[i].length() );
             }
             size_t end = channelName.length();
             while( end > 0 && ::isspace( channelName[end-1] ) )
               --end;
             size_t begin = 0;
             while( begin < end && ::isspace( channelName[begin] ) )
               ++begin;
             gChannelNames.push_back( channelName.substr( begin, end - begin ) );
           }
           if( !gSilent )
           {
             cout << "Channel names:";
             for( size_t i = 0; i < gChannelNames.size(); ++i )
               cout << " " << gChannelNames[i];
             cout << endl;
           }
         }
       }
     }
    }
 else if (pMsg->m_wCode == DataType_EegData)
    {
    if (pMsg->m_wRequest == DataTypeRaw16bit)
       {
       // printf("Processing 16 bit raw data block\n");
       // make sure the body has the right length - even if we're not processing the data
       bciassert((int)(pMsg->m_dwSize) == (int)(m_BasicInfo.nEegChan+m_BasicInfo.nEvtChan)*m_BasicInfo.nBlockPnts*m_BasicInfo.nDataSize);
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


