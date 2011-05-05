////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: shane@rppl.com
// Description: ADC class to relay BCI-formatted packets from the Grapevine
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////

#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIError.h"

#include "GrapeVineADC.h"

using namespace std;

// Register the source class with the framework.
RegisterFilter( GrapeVineADC, 1 );

GrapeVineADC::GrapeVineADC() :
    mSourceCh(0),
    mSampleBlockSize(0)
{
  BEGIN_PARAMETER_DEFINITIONS

    "Source int SourceCh= 128 32 1 128"					" // Number of digitized channels",
    "Source int SampleBlockSize= 10 10 1 64"			" // Samples transmitted per block",
    "Source int SamplingRate= 500Hz 200Hz 200Hz 2000Hz" " // Sampling rate (200, 500, 1000, or 2000)",

//  "Source list ChannelNames= 34 "
//  "O2 O1 Oz Pz P4 CP4 P8 C4 TP8 T8 "
//  "P7 P3 CP3 CPz Cz FC4 FT8 TP7 C3 FCz "
//  "Fz F4 F8 T7 FT7 FC3 F3 FP2 F7 FP1 "
//  "HEOR HEOL VEOL VEOU"                               " // Channel Names",

    "Source list ChannelNames= 128 "
    "01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 "
    "33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 "
    "65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 "
    "97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127 128 "
    " // Channel Names",

    "Source list SourceChOffset= 128 "
    "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "
    "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "
    "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "
    "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "
    " // Channel Offset",

    "Source list SourceChGain= 128 "
    "1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 "
    "1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 "
    "1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 "
    "1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 "
    " // Channel Gain",

    "Source int Priority= 1 1 0 3 "                     "// CPU priority for source module: "
    "0: Normal, 1: AboveNormal, 2: High, 3: Realtime (enumeration)"

  END_PARAMETER_DEFINITIONS

    // initialize to invalid state to prevent Halt() from closing an invalid socket
    mGvBciSocket = INVALID_SOCKET;
}

GrapeVineADC::~GrapeVineADC() { Halt(); }


void
GrapeVineADC::Preflight( const SignalProperties&,SignalProperties& Output ) const
{
    double samplingRate = Parameter( "SamplingRate" ).InHertz();
    if (samplingRate==200) bciout << "Verify Grapevine set to deliver 200 Samples/Sec" << endl;
    else if (samplingRate==500)  bciout << "Verify Grapevine set to deliver 500 Samples/Sec"  << endl;
    else if (samplingRate==1000) bciout << "Verify Grapevine set to deliver 1000 Samples/Sec" << endl;
    else if (samplingRate==2000) bciout << "Verify Grapevine set to deliver 2000 Samples/Sec" << endl;
    else { bcierr << "Source Module only supports Grapevine Sample rates of 200, 1000, and 2000 sps." << endl; return; }

    if ( Parameter("SourceCh") > 128 ) { bcierr << "Source Module supports maximum of 128 channels" << endl; return; }

    Output = SignalProperties( Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), SignalType::float32 );
}

void
GrapeVineADC::Initialize( const SignalProperties&, const SignalProperties& )
{
    this->Halt();

    mSourceCh = Parameter("SourceCh");
    mSampleBlockSize = Parameter("SampleBlockSize");

    // set packet buffer to all zeros, also provides flat traces if un-filled parts of buffer are sent on
    mGvBciPacket.sequence  = 0;
    mGvBciPacket.timeStamp = 0;
    mGvBciPacket.nChan     = 0;
    mGvBciPacket.nSamp     = 0;
    for( int ch = 0; ch < (sizeof(mGvBciPacket.data)/sizeof(float)); ++ch ) mGvBciPacket.data[ch] = 0.0;

    mGvSleepCount   = 0;
    mGvBciSampIndex = 0;
    mGvBciLastSeq   = 0;

    DWORD Priority;
    switch ((int) Parameter( "Priority" ))
    {
        case 0  : Priority=NORMAL_PRIORITY_CLASS;        break;
        case 1  : Priority=ABOVE_NORMAL_PRIORITY_CLASS;  break;
        case 2  : Priority=HIGH_PRIORITY_CLASS;          break;
        case 3  : Priority=REALTIME_PRIORITY_CLASS;      break;
        default : Priority=ABOVE_NORMAL_PRIORITY_CLASS;
    }
    SetPriorityClass(GetCurrentProcess(), Priority);

    OpenSocket();
}


void
GrapeVineADC::StartRun()
{
}


void
GrapeVineADC::Process( const GenericSignal&, GenericSignal& Output )
{
    // Outer loop runs until a until the Output is filled with a complete sample block.
    // GvBciPackets from the instrument UDP network may be packed with more than one sample
    // for transmission density (esp at higher sample rates), and the number of samples is
    // likely different than the number of samples needed for each BCI2000 sample block.
    int sampleBlockIndex = 0;
    while( sampleBlockIndex < mSampleBlockSize )
    {
        if (mGvBciSampIndex >= mGvBciPacket.nSamp) // Get next pkt if no more blocks in pkt buffer to process
        {
            if (mGvBciSocket==INVALID_SOCKET) return;   // force abort if Halt() is called

            int udpPacketSize = recv(mGvBciSocket, (char*)&mGvBciPacket, sizeof(mGvBciPacket), 0);
            if ( udpPacketSize > 0 )  // check for packet received
            {
                mGvSleepCount = 0;    // reset the sleep counter
                mGvBciSampIndex = 0;  // rewind index to process sample blocks within mGvBciPacket

                // Badly formed packets should be very rare events, flag error if one is caught
                if (udpPacketSize != mGvBciPacket.GetPacketSize()) bcierr << "Bad UDP packet recieved " << endl;

                // Report packets recieved out of order (packet drops in instrument network)
                if ( mGvBciLastSeq && (mGvBciPacket.sequence != (mGvBciLastSeq+1)) )
                    bciout << "Out of Order Packet, " << (mGvBciPacket.sequence - mGvBciLastSeq - 1)
                           << " packets skipped, " << " at time index " << mGvBciPacket.timeStamp << endl;
                 mGvBciLastSeq = mGvBciPacket.sequence;
            }
            else    // no packet waiting to be recieved or socket error (handle them the same here)
            {
                mGvBciPacket.nSamp = 0;     // reset to ensure no samples in packet buffer are processed

                // sleep current thread to wait for next packet
                if (++mGvSleepCount <= GV_SLEEP_CNT_MAX) Sleep(GV_SLEEP_TIME_MS);
                else // too much sleep, clear Output and return to let interfaces update and program exit if needed
                {
                    for( int ch = 0; ch < mSourceCh; ++ch )
                        for( int smp = 0; smp < mSampleBlockSize; ++smp) Output( ch, smp ) = 0.0;
                    mGvSleepCount = 0;  // reset sleep counter
                    mGvBciLastSeq = 0;  // reset sequence counter to prevent errors if acq system is restarted
                    return;
                }
            }
         }
        else // unprocessed sample block in packet buffer, move it into the Output sample block
        {
            for( int ch = 0; ch < mSourceCh; ++ch )
                Output( ch, sampleBlockIndex ) = mGvBciPacket.GetSample(mGvBciSampIndex,ch);
            ++sampleBlockIndex;
            ++mGvBciSampIndex;
        }
    }
}

void
GrapeVineADC::StopRun()
{

}


void
GrapeVineADC::Halt()
{
    if (mGvBciSocket != INVALID_SOCKET)
    {
        shutdown(mGvBciSocket,2);       // SD_BOTH=2, seems undefined in winsock.h
        closesocket(mGvBciSocket);
        mGvBciSocket = INVALID_SOCKET;
    }

    // restore thread priority to normal levels
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
}


void GrapeVineADC::OpenSocket()
{
    // Open the socket (in win32, framework will call WSAStartup and WSACleanup)
    mGvBciSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (mGvBciSocket==INVALID_SOCKET) { bcierr << "Unable to Open Socket" << endl;  return; }

    // set broadcast mode and dont-route option for socket
    BOOL optVal = TRUE;
    int  optLen = sizeof(optVal);
    if ( (setsockopt(mGvBciSocket, SOL_SOCKET, SO_BROADCAST, (char*)&optVal, optLen) == SOCKET_ERROR)
      || (setsockopt(mGvBciSocket, SOL_SOCKET, SO_DONTROUTE, (char*)&optVal, optLen) == SOCKET_ERROR) )
    {
        closesocket(mGvBciSocket);
        bcierr << "Unable to Set Socket Modes" << endl;
        return;
    }

    // set input data buffer size to a cool Meg
    int optBufSize = 1048576;
        optLen = sizeof(optBufSize);
    if ( setsockopt(mGvBciSocket, SOL_SOCKET, SO_RCVBUF, (char*)&optBufSize, optLen) == SOCKET_ERROR )
    {
        closesocket(mGvBciSocket);
        bcierr << "Unable to Set Input Buffer Size" << endl;
        return;
    }

    // set the socket for non-blocking operation
    u_long argVal = 1;
    if ( ioctlsocket(mGvBciSocket, FIONBIO, &argVal) == SOCKET_ERROR )
    {
        closesocket(mGvBciSocket);
        bcierr << "Unable to Set Socket for Non-Blocking Operation" << endl;
        return;
    }

    // bind socket to instrument network card with address btn 192.168.42.128 and 192.168.42.254
    SOCKADDR_IN instSockAddr;
    instSockAddr.sin_family		 = AF_INET;
    instSockAddr.sin_port		 = htons(17454); // port ID for GvBciPackets from GV NIP
    instSockAddr.sin_addr.s_addr = inet_addr("192.168.42.127");
    while ( ++(instSockAddr.sin_addr.S_un.S_un_b.s_b4) < 255 )
        if (bind(mGvBciSocket, (struct sockaddr FAR *)&instSockAddr, sizeof(instSockAddr))==0) break;
    if ( (instSockAddr.sin_addr.S_un.S_un_b.s_b4) == 255 )
    {
        closesocket(mGvBciSocket);
        bcierr << "Unable to Bind Socket" << endl;
        return;
    }
    else bciout << "Bound to Socket 192.168.42." << (int)(instSockAddr.sin_addr.S_un.S_un_b.s_b4) << endl;
}
