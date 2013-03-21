////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: shane@rppl.com
// Description: ADC class to relay BCI-formatted packets from the Grapevine
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
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
#include "errno.h"

#include <stdio.h>
#include <iomanip>

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

    "Source int AcquisitionMode= 0 0 0 2 "
    "// Data Acquisition Mode: "
    " 0: acquisition,"
    " 1: single-sweep impedance,"
    " 2: continuous impedance,"
    " (enumeration)",
    
    "Source int ImpedanceCurrent= 1 1 0 3 "
    "// Impedance Current <nA pk-pk>: "
    " 0: <3> Implant microelectrode,"
    " 1: <10> Implant macroelectrode,"
    " 2: <30> Surface hi-Z electrode,"
    " 3: <100> Surface lo-Z electrode,"
    " (enumeration)",
    
    "Source int ImpedanceSpeed= 0 0 0 2 "
    "// Impedance Speed: "
    " 0: Fast Estimate,"
    " 1: More Accurate,"
    " 2: Most Accurate,"
    " (enumeration)",
    
    "Source int Priority= 1 1 0 3 "
    "// CPU priority for source module: "
    "0: Normal, 1: AboveNormal, 2: High, 3: Realtime (enumeration)",
    
    "Source matrix Impedances= 1 1 not%20measured%20yet % % % "
    "// impedances measured by front ends in kohm "
    "--rows represent front ends, columns represent channels",

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
    else if (samplingRate==1200) bciout << "Verify Grapevine set to deliver 1200 Samples/Sec" << endl;
    else if (samplingRate==2000) bciout << "Verify Grapevine set to deliver 2000 Samples/Sec" << endl;
    else { bcierr << "Source Module only supports Grapevine Sample rates of 200, 1000, 1200, and 2000 sps." << endl; return; }

    if ( Parameter("SourceCh") > 128 ) { bcierr << "Source Module supports maximum of 128 channels" << endl; return; }

    Output = SignalProperties( Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), SignalType::float32 );
    
    Parameter( "Impedances" );  // touch Impedances property so that BCI2k thinks it's checked
}

void
GrapeVineADC::Initialize( const SignalProperties&, const SignalProperties& )
{
    // close socket if it is open
    CloseSocket();

    mSourceCh = Parameter("SourceCh");
    mSampleBlockSize = Parameter("SampleBlockSize");

    // set packet buffer to all zeros, also provides flat traces if un-filled parts of buffer are sent on
    mGvBciData.sequence  = 0;
    mGvBciData.timeStamp = 0;
    mGvBciData.nChan     = 0;
    mGvBciData.nSamp     = 0;
    for( int ch = 0; ch < (sizeof(mGvBciData.data)/sizeof(float)); ++ch ) mGvBciData.data[ch] = 0.0;

    mGvSleepCount   = 0;
    mGvBciLastSeq   = 0;

#ifdef _WIN32

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

#else // _WIN32
    
    int sched_policy;
    sched_param param;
    if (pthread_getschedparam(pthread_self(), &sched_policy, &param) != 0)
    { bcierr << "Cannot get scheduler parameters" << endl;  return; }

    int prio_min = sched_get_priority_min( sched_policy );
    int prio_max = sched_get_priority_max( sched_policy );
    if (prio_min == -1 || prio_max == -1)
    { bcierr << "Cannot determine scheduler priority range" << endl;  return; }
    
    switch ((int) Parameter( "Priority" ))
    {
        case 0  : param.sched_priority = ((3*prio_min) + (1*prio_max)) / 4;   break;
        case 1  : param.sched_priority = ((2*prio_min) + (2*prio_max)) / 4;   break;
        case 2  : param.sched_priority = ((1*prio_min) + (3*prio_max)) / 4;   break;
        case 3  : param.sched_priority = ((0*prio_min) + (4*prio_max)) / 4;   break;
        default : param.sched_priority = ((2*prio_min) + (2*prio_max)) / 4;   break;
    }

    if (pthread_setschedparam(pthread_self(), sched_policy, &param))
    { bcierr << "Cannot set thread priority" << endl;  return; }

#endif // _WIN32
    
    // fire up the socket
    OpenSocket();
    
    // if impedance mode, resize impedance array and initialize to zero
    mAcqMode = Parameter("AcquisitionMode");
    bciout << "Acquisition Mode " << mAcqMode << endl;
    if (mAcqMode) // true for impedance modes
    {
        MutableParamRef paramImpedances = Parameter( "Impedances" );
        unsigned FrontEndCnt = (mSourceCh + (CHANNELS_PER_FRONT_END-1)) / CHANNELS_PER_FRONT_END;
        paramImpedances->SetDimensions( FrontEndCnt, CHANNELS_PER_FRONT_END );
        for( size_t i=0; i<FrontEndCnt; ++i )
            for( size_t j=0; j<CHANNELS_PER_FRONT_END; ++j )
            {
                ostringstream oss;
                oss << "0";
                paramImpedances( i, j ) = oss.str();
            }
        
        // and commend NIP into impedance measurement mode
        int impCycles = Parameter("ImpedanceSpeed");
        int impCurrent = Parameter("ImpedanceCurrent");
        GvBciCommand gvBciCmdPkt = { mAcqMode, impCurrent, ((impCycles + 1) * IMP_NCYCS_MIN) };

        sockaddr nipAddr = { 0 };
        ((sockaddr_in *) &nipAddr)->sin_family      =  AF_INET;
        ((sockaddr_in *) &nipAddr)->sin_port        =  htons( GV_PORT_BCI_TO_NIP );
        ((sockaddr_in *) &nipAddr)->sin_addr.s_addr =  inet_addr( "192.168.42.1" );
        if (!sendto( mGvBciSocket, &gvBciCmdPkt, sizeof(gvBciCmdPkt), 0, &nipAddr, sizeof(nipAddr)))
            bcierr << "Unable to send BCI impedance control packet" << endl;
    }
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
        if (mGvBciSocket==INVALID_SOCKET) return;   // force abort if Halt() is called

        int udpPacketSize = recv(mGvBciSocket, (char*)&mGvBciData, sizeof(mGvBciData), 0);
        if ( udpPacketSize > 0 )  // check for packet received
        {
            // Badly formed packets should be very rare events, flag error if one is caught
            if (udpPacketSize != mGvBciData.GetPacketSize()) bcierr << "Bad UDP packet recieved " << endl;
            
            // Report packets recieved out of order (packet drops in instrument network)
            if ( mGvBciLastSeq && (mGvBciData.sequence != (mGvBciLastSeq+1)) )
                bciout << "Out of Order Packet, " << (mGvBciData.sequence - mGvBciLastSeq - 1)
                << " packets skipped, " << " at time index " << mGvBciData.timeStamp << endl;
            mGvBciLastSeq = mGvBciData.sequence;
            
            if (mGvBciData.nSamp)
            {
                // move the packet contents to the block index buffer
                for (unsigned smp=0; smp < mGvBciData.nSamp; ++smp)
                {
                    for( int ch = 0; ch < mSourceCh; ++ch )
                        Output( ch, sampleBlockIndex ) = mGvBciData.GetSample(smp,ch);
                    ++sampleBlockIndex;
                }
            }
            else // if nSamp == 0, packet contains impedance measurements
            {
                MutableParamRef paramImpedances = Parameter( "Impedances" );
                for( unsigned ch=0; ch<mSourceCh; ++ch)
                {
                    ostringstream oss;
                    oss << std::fixed << std::setprecision(1);
                    oss << (0.001f * mGvBciData.GetSample(1,ch));
                    size_t i = ch / CHANNELS_PER_FRONT_END;
                    size_t j = ch % CHANNELS_PER_FRONT_END;
                    paramImpedances( i, j ) = oss.str();
                }
            }

            // reset the sleep counter
            mGvSleepCount = 0;
        }
        else    // no packet waiting to be recieved or socket error (handle them the same here)
        {
            // sleep current thread to wait for next packet
            if (++mGvSleepCount <= GV_SLEEP_CNT_MAX) Sleep(GV_SLEEP_TIME_MS);
            else // too much sleep, clear Output and return to let interfaces update and program exit if needed
            {
                for( int ch = 0; ch < mSourceCh; ++ch )
                    for( int smp = 0; smp < mSampleBlockSize; ++smp) Output( ch, smp ) = 0.0;
                mGvSleepCount = 0;  // reset sleep counter
                return;
            }
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
    // make sure socket is closed
    CloseSocket();
    
    // restore thread priority to normal levels

#ifdef _WIN32

    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

#else 

    int sched_policy;
    sched_param param;
    if (pthread_getschedparam(pthread_self(), &sched_policy, &param) == 0)
    {
        param.sched_priority = sched_get_priority_min( sched_policy );
        int status = pthread_setschedparam(pthread_self(), sched_policy, &param);
    }

#endif
    
}


void GrapeVineADC::OpenSocket()
{
    
#ifdef _WIN32

    // Open the socket (in win32, framework will call WSAStartup and WSACleanup)
    mGvBciSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (mGvBciSocket==INVALID_SOCKET) { bcierr << "Unable to Open Socket" << endl;  return; }
    
    // bind socket to instrument network card with address btn 192.168.42.128 and 192.168.42.254
    unsigned host = 127;
    while ((++host) < 255)
    {
        struct sockaddr_in instSockAddr;
        instSockAddr.sin_family		 = AF_INET;
        instSockAddr.sin_port		 = htons( GV_PORT_NIP_TO_BCI );
        instSockAddr.sin_addr.s_addr = htonl( (192 << 24) | (168 << 16) | (42 << 8) | host );
        if (bind(mGvBciSocket, (struct sockaddr *)&instSockAddr, sizeof(instSockAddr)) == 0) break;
    }
     
    if (host < 255) bciout << "Bound to Socket 192.168.42." << host << endl;
    else {  CloseSocket();  bcierr << "Unable to Bind Socket (" << strerror(errno) << ")" << endl;  return;  }
    
    // set broadcast mode and dont-route option for socket
    int optTrue = 1;
    int optBufSize = 1048576;
    int optLen = sizeof(int);
    if ( (setsockopt(mGvBciSocket, SOL_SOCKET, SO_BROADCAST, (char *)&optTrue, optLen) == SOCKET_ERROR)
      || (setsockopt(mGvBciSocket, SOL_SOCKET, SO_DONTROUTE, (char *)&optTrue, optLen) == SOCKET_ERROR)
      || (setsockopt(mGvBciSocket, SOL_SOCKET, SO_RCVBUF, (char *)&optBufSize, optLen) == SOCKET_ERROR) )
    {   CloseSocket();  bcierr << "Unable to Set Socket Parameters" << endl;  return;    }

    // set the socket for non-blocking operation
    u_long argVal = 1;
    if ( ioctlsocket(mGvBciSocket, FIONBIO, &argVal) == SOCKET_ERROR )
    {   CloseSocket();  bcierr << "Unable to Set Socket for Non-Blocking Operation" << endl;  return;  }
    
#else 
    
    // Open the socket (in win32, framework will call WSAStartup and WSACleanup)
    mGvBciSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (mGvBciSocket==INVALID_SOCKET) { bcierr << "Unable to Open Socket" << endl;  return; }
    
    struct sockaddr_in instSockAddr;
    instSockAddr.sin_family		 = AF_INET;
    instSockAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    instSockAddr.sin_port		 = htons( GV_PORT_NIP_TO_BCI );
    if (bind(mGvBciSocket, (struct sockaddr *)&instSockAddr, sizeof(instSockAddr)) == 0)
        bciout << "Socket bound to all interfaces" << endl;
    else {   CloseSocket();  bcierr << "Unable to Bind Socket (" << strerror(errno) << ")" << endl;  return;  }
    
    // set broadcast mode and dont-route option for socket
    int optTrue = 1;
    int optBufSize = 1048576;
    int optLen = sizeof(int);
    if ( setsockopt(mGvBciSocket, SOL_SOCKET, SO_BROADCAST, &optTrue, optLen)
      || setsockopt(mGvBciSocket, SOL_SOCKET, SO_DONTROUTE, &optTrue, optLen)
      || setsockopt(mGvBciSocket, SOL_SOCKET, SO_RCVBUF, &optBufSize, optLen) )
    {   CloseSocket();  bcierr << "Unable to Set Socket Parameters" << endl;  return;    }

    // set the socket for non-blocking operation
    int flags = fcntl(mGvBciSocket, F_GETFL, 0);
    if (fcntl(mGvBciSocket, F_SETFL, flags | O_NONBLOCK))
    {   CloseSocket();  bcierr << "Unable to Set Socket for Non-Blocking Operation" << endl;  return;  }
    
#endif // _WIN32
    
}


void GrapeVineADC::CloseSocket()
{
    
#ifdef _WIN32
    
    // make sure socket is closed
    if (mGvBciSocket != INVALID_SOCKET)
    {
        shutdown(mGvBciSocket,2);       // SD_BOTH=2, seems undefined in winsock.h
        closesocket(mGvBciSocket);
        mGvBciSocket = INVALID_SOCKET;
    }
    
#else
    
    // make sure socket is closed
    if (mGvBciSocket != INVALID_SOCKET)
    {
        shutdown(mGvBciSocket,SHUT_RDWR);
        close(mGvBciSocket);
        mGvBciSocket = INVALID_SOCKET;
    }
    
#endif

}


