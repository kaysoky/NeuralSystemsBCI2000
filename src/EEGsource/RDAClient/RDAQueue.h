////////////////////////////////////////////////////////////////////////////////
//
// File: RDAQueue.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Jan 3, 2003
//
// Description: A class that encapsulates connection details of the BrainAmp
//              RDA socket interface.
//              Like the RDA protocol itself, this class is
//              naive about type and endianness issues.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef RDAQUEUEH
#define RDAQUEUEH

#include <queue>
#include <winsock.h>

// #defining RDA_SHORT will switch to the older 16bit integer protocol.
#if( !defined( RDA_FLOAT ) && !defined( RDA_SHORT ) )
# define RDA_FLOAT 1
#endif

#ifdef RDA_FLOAT
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
      int                 blockDuration;
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
