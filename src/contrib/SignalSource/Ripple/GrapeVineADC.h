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

    // Proprietary constants and structures to communicate between GV NIP and this source module
    
    enum GV_CMD_MODE
    {
        CMD_MODE_NORMAL = 0,    // normal mode
        CMD_MODE_IMP_SNGL,      // check impedances once, then go to normal
        CMD_MODE_IMP_CONT       // check impedances continually
    };
    
    enum GV_IMP_ILIST           // impedance test currents (pk-pk)
    {                           
        IMP_ILIST_3 = 0,        //   3 nA
        IMP_ILIST_10,           //  10 nA - use for micro and implanted electrodes
        IMP_ILIST_30,           //  30 nA
        IMP_ILIST_100           // 100 nA - use for surface electrodes
    };

    enum GV_IMP_NCYCS
    {                               
        IMP_NCYCS_FAST     =  20,   // number of sine cycles (1ms each)
        IMP_NCYCS_MORE_ACC =  70,   // used to measure each channel
        IMP_NCYCS_MOST_ACC = 150,   // (cycles for combo box settings)
    };
    
    static const unsigned FRONT_ENDS_MAX = 4;
    static const unsigned CHANNELS_PER_FRONT_END = 32;
    static const unsigned CHANNELS_MAX = FRONT_ENDS_MAX * CHANNELS_PER_FRONT_END;
    
    static const unsigned GV_SLEEP_TIME_MS = 5;  // time used for Sleep commands when waiting on packets.
    static const unsigned GV_SLEEP_CNT_MAX = 20; // maximum consecutive Sleep commands before giving up
    
    static const unsigned GV_PORT_NIP_TO_BCI = 17454;
    static const unsigned GV_PORT_BCI_TO_NIP = 17455;
    
    
    #pragma pack(push,1)
    
    struct GvBciCommand         // Packets sent to NIP to set BCI mode
    {
        uint32_t mode;          // normal vs impedance mode from GV_CMD_MODE
        uint32_t iListEntry;    // output test current (entry id from GV_IMP_ILIST)
        uint32_t cycPerChan;    // sine cycles per channel per measurement (within IMP_NCYCS_*)
    };
    
    struct GvBciData            // Packets received from NIP with signal or impedance data
    {
        uint32_t sequence;      // sequence counter for detecting dropped packets
        uint32_t timeStamp;     // current NIP timestamp
        uint32_t nSamp;         // number of samples in packet (0 = pkt contains impedance data)
        uint32_t nChan;         // number of channels within the packet
        float    data[256];     // floating point data[nSamp][nChan]
        inline float GetSample( unsigned samp, unsigned chan ) { return data[ (samp*nChan) + chan ]; }
        inline unsigned GetPacketSize() { return (4 * sizeof(uint32_t))
                                               + (nChan * (nSamp ? nSamp : 1) * sizeof(float)); }
    };

    #pragma pack(pop)

    // actual internal private members
    
    void        OpenSocket(void);
    void        CloseSocket(void);

    int			mSampleBlockSize;   // number of samples per bci2000 output sample block
    int         mSourceCh;          // number of channels per bci2000 output sample block
    int         mAcqMode;

    SOCKET      mGvCtlSocket;
    SOCKET		mGvBciSocket;       // Socket for receiving UDP GvBciPackets from Grapevine
    unsigned    mGvSleepCount;      // number of consecutive Sleseps waiting for packets
    uint32_t    mGvBciLastSeq;      // sequence number of last GvBciPacket recieved

    GvBciData   mGvBciData;         // buffer for holding currently processed GvBciPacket

    GenericVisualization mVis;
};

#endif // GRAPEVINE_ADC_H

