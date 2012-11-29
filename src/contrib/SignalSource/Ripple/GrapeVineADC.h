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
#ifndef GRAPEVINE_ADC_H
#define GRAPEVINE_ADC_H

#include "GenericADC.h"

#include "SockStream.h"

// Portability for non-windows builds using unix sockets and posix usleep
#ifndef _WIN32
#include <fcntl.h>
#include <arpa/inet.h>
#define INVALID_SOCKET -1           // to duplicate value used in Windows
#define Sleep(s) usleep(1000*(s))
#endif


#define GRAPEVINE_SF 30000	// GrapeVine Sampling Frequency

static const unsigned GV_SLEEP_TIME_MS = 5;  // time used for Sleep commands when waiting on packets.
static const unsigned GV_SLEEP_CNT_MAX = 20; // maximum consecutive Sleep commands before giving up

class GrapeVineADC : public GenericADC
{
public:
    GrapeVineADC();
    virtual      ~GrapeVineADC();

    virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void Initialize( const SignalProperties&, const SignalProperties& );
    virtual void StartRun();
    virtual void StopRun();
    virtual void Process( const GenericSignal&, GenericSignal& );
    virtual void Halt();


private:

    #pragma pack(push,1)
    struct GvBciPacket {
        uint32_t sequence;
        uint32_t timeStamp;
        uint32_t nSamp;
        uint32_t nChan;
        float    data[256];
        inline unsigned GetPacketSize() { return (4*sizeof(uint32_t)) + (nChan*nSamp*sizeof(float)); }
        inline float GetSample( unsigned samp, unsigned chan ) { return data[ (samp*nChan) + chan ]; }
    };
    #pragma pack(pop)

    void        OpenSocket(void);
    void        CloseSocket(void);

    int			mSampleBlockSize;   // number of samples per bci2000 output sample block
    int         mSourceCh;          // number of channels per bci2000 output sample block

    SOCKET		mGvBciSocket;       // Socket for receiving UDP GvBciPackets from Grapevine
    unsigned    mGvSleepCount;      // number of consecutive Sleseps waiting for packets
    uint32_t    mGvBciLastSeq;      // sequence number of last GvBciPacket recieved

    GvBciPacket mGvBciPacket;       // large buffer for holding currently processed GvBciPacket
};

#endif // GRAPEVINE_ADC_H

