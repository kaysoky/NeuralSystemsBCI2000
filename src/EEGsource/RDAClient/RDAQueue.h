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
////////////////////////////////////////////////////////////////////////////////
#ifndef RDAQUEUEH
#define RDAQUEUEH

#include <queue>
#include <winsock.h>

class RDAQueue : public std::queue<short>
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
    const short& front();
    operator bool() const { return failstate == ok; }
    bool is_open() const  { return socketHandle != NULL; }
    const RDAInfo& info() const { return connectionInfo; }

  private:
    enum RDAMessageType
    {
      RDAStart = 1,
      RDAData = 2,
      RDAStop = 3,
    };
    static const size_t initialBufferSize = 1024;
    static const int blockDurationGuess = 40000; // microseconds
    static const int samplingIntervalGuess = 10000;
    static const int numChannelsGuess = 8;
    static const int channelResolutionGuess = 1000;
    static const int RDAPortNumber = 51234;

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
