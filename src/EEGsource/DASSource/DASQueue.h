////////////////////////////////////////////////////////////////////////////////
//
// File: DASQueue.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Sep 23, 2003
//
// Description: A class that interfaces with A/D boards supported by
//              MeasurementComputing's Universal Library. 
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef DASQUEUEH
#define DASQUEUEH

#include <queue>
#include <windows.h>

class DASQueue : public std::queue<short>
{
  public:
    struct DASInfo
    {
      long  boardNumber,
            samplingRate,
            sampleBlockSize,
            numChannels;
      float adRangeMin, adRangeMax;
    };
    enum
    {
      ok = 0,
      initFail = 1 << 0,
      connectionFail = 1 << 1,
      memoryFail = 1 << 2,
    };
    DASQueue();
    ~DASQueue();
    void open( const DASInfo& inInfo );
    void close();
    void clear()          { c.clear(); mFailstate = ok; mShouldBeOpen = false; }
    const short& front();
    operator bool() const { return mFailstate == ok; }
    bool is_open() const  { return mShouldBeOpen && mFailstate == ok; }

  private:
    static const int cTimeoutFactor = 2;  // How many sample block durations
                                          // the timeout should be.
    static const int cBlocksInBuffer = 4; // How many sample blocks shall
                                          // fit into the sample buffer.

    int     mFailstate,
            mBoardNumber,
            mFreqMultiplier,
            mChannels,
            mHWChannels;
    USHORT* mDataBuffer;
    long    mDataBufferSize,
            mReadCursor,
            mWriteCursor;
    bool    mShouldBeOpen;
    DWORD   mTimeoutInterval;
    HANDLE  mDataAvailableEvent;

    void ReceiveData();
    bool IgnoredSample( long inIndex );
    static void CALLBACK BoardEventCallback( int inBoardNumber,
                         unsigned inEventType, unsigned inEventData, void* inUserData );
};

#endif // DASQUEUEH
