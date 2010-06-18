////////////////////////////////////////////////////////////////////////////////
// $Id: vAmpThread.h 2032 2008-06-26 17:11:32Z mellinger $
// Author: jadamwilson2@gmail.com
// Description: A class that encapsulates a data acquisition thread for
//   vAmp devices
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef VAMP_THREAD_H
#define VAMP_THREAD_H

#include <windows.h>
#include <sstream>
#include <valarray>
#include <vector>
#include <map>
#include <set>
#include <stdio.h>
#include "GenericSignal.h"
#include "defines.h"
#include "OSThread.h"
#include "FirstAmp.h"
#include "vAmpChannelInfo.h"
#include "vAmpDefines.h"
#include "IIRFilter.h"

class vAmpThread : public OSThread
{
  static const int cBlocksInRingBuffer = 10;
  static const unsigned short cDisplayUpdateTime = 333;
 public:
  vAmpThread(
    int inBlockSize,
    float sampleRate,
    int decimate,
    const std::vector<int>& chList,    
    const std::vector<int>& devList,
    int mode,
    float hpCorner
  );
  virtual ~vAmpThread();

  float ExtractData(int ch, int sample);
  std::vector< std::vector<float> > GetImpedances();
  std::string GetLastErr() const {return mLastErr.str();}
  std::string GetLastWarning();
  bool ok() const {return mOk;}
  void AdvanceReadBlock();
  HANDLE acquireEventRead;

  // Lock for data and message strings.
  void Lock() { while( mLock ) Sleep( 0 ); mLock = true; }
  void Unlock() { mLock = false; }

 private:
  volatile bool mLock;

 private:
    virtual int Execute();

    void DisplayBCI2000Logo( int id );
    void ClearAmpDisplay( int id );
    unsigned int DisplayImpedances( int id, const std::vector<float>& );
    void ValueToText( float inValue, std::string& outText, TColor& outColor );
    Graphics::TBitmap* NewBitmap() const;

    int   mBlockSize,
          mTimeout,
          mBufSize,
          mNumPoints,
          mWriteCursor,
          mReadCursor,
          mRingBufferSize,
          mDecimate,
          mNumChannels,
          mImpInitState;//contains the feedbackvalue from Impedance initialisation;
    float mSampleRate;
    unsigned short mPrevTime;
	
    std::ostringstream mLastErr,
                       mWarnings;
    std::vector<int> mChList;
    std::vector<unsigned int> mImpBuf;
    int mMode;
  
    GenericSignal mDataBuffer, mDataOutput;

    std::vector< std::vector<float> > mImpArray;
    std::valarray< std::valarray<float> > mTrigBuffer; //mdatabuffer[device][ch][sample]
    float *mBuffer;
    HANDLE mEvent,
           mDev;
    bool mOk;
    bool mIs8Channel;
   
    unsigned int mNumDevices;
    int mDevIds[MAX_ALLOWED_DEVICES];
    std::vector<int> mDevList;
    int m_nChannelMode;
    int m_nEEGChannels;
    int m_nAUXChannels;    
    unsigned int mDataCounterErrors;    
    
    int mStartMode;
    int mBufferSize;
    int m_nMaxPoints;
    t_faInformation m_DeviceInfo[MAX_ALLOWED_DEVICES];      // Device info.
    t_faProperty    m_DeviceProp[MAX_ALLOWED_DEVICES];      // Channel properties.
    std::vector<CChannelInfo>
                    m_tblChanInfo[MAX_ALLOWED_DEVICES];

    t_faDataFormatMode20kHz* m_tblMaxBuf4[MAX_ALLOWED_DEVICES];     // 1 read cycle buffer (highspeed, 4 ch) + 2 add. samples.
    t_faDataModel8*          m_tblMaxBuf8[MAX_ALLOWED_DEVICES];     // 1 read cycle buffer of 8 channel system + 2 add. samples.
    t_faDataModel16*         m_tblMaxBuf16[MAX_ALLOWED_DEVICES];    // 1 read cycle buffer of 16 channel system + 2 add. samples.
   
    t_faDataModeSettings mFastSettings[MAX_ALLOWED_DEVICES];
    t_faDataMode mDataMode;
    bool mHighSpeed;
    std::set<int> mDigChs;

    int ReadData(int nDeviceId, char *pBuffer, int nReadLen);
    IIRFilter<float> mFilter;
    typedef Ratpoly<FilterDesign::Complex> TransferFunction;
};

#endif // VAMP_THREAD_H
//---------------------------------------------------------------------------

