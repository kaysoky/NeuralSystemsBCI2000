////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates connection details of the BrainAmp
//              RDA socket interface.
//              Like the RDA protocol itself, this class is
//              naive about type and endianness issues.
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
#ifndef RDAQUEUEH
#define RDAQUEUEH

#include <queue>
#include <winsock.h>

#ifndef RDA_FLOAT
# define RDA_FLOAT 1
#endif

#if RDA_FLOAT
// RDAPort Numbers: 51244 -> float data, 51234 -> short data
# define RDAPORTNUMBER 51244
#else
# define RDAPORTNUMBER 51234
#endif

typedef float queue_type;

class RDAQueue : public std::queue<queue_type>
{
  public:
    struct RDAInfo
    {
      size_t              numChannels;
      double              samplingInterval;
      ULONG               blockNumber;
      double              blockDuration;
      std::vector<double> channelResolutions;
    };
    enum
    {
      ok = 0,
      netinitFail = 1 << 0,
      connectionFail = 1 << 1,
      memoryFail = 1 << 2,
    };
    RDAQueue();
    ~RDAQueue();
    void open( const char* inHostName );
    void close();
    void clear()          { c.clear(); failstate = ok; }
    const queue_type& front();
    operator bool() const { return failstate == ok; }
    bool is_open() const  { return socketHandle != NULL; }
    const RDAInfo& info() const { return connectionInfo; }

  private:
    enum RDAMessageType
    {
      RDAStart = 1,
      RDAData = 2,
      RDAStop = 3,
      RDAData32 = 4,
    };
    static const size_t initialBufferSize = 1024;
    static const int blockDurationGuess = 40000; // microseconds
    static const int startBlockTimeout = 5; // timeout in seconds when waiting for the start block
    static const int blockNumberMask = 0x00ffffff; // what we consider significant,
                                                   // i.e. wrap-around safe
    static const int RDAPortNumber = RDAPORTNUMBER;        // 51244 -> float data, 51234 -> short data

    void ReceiveData();
    void GetServerMessage();

    size_t bufferSize;
    char*  receiveBuffer;
    int    failstate;

    RDAInfo        connectionInfo;
    RDAMessageType lastMessageType;

    SOCKET socketHandle;
};

#endif // RDAQUEUEH
