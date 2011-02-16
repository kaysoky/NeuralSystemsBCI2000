////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: jadamwilson2@gmail.com
// Description: A class that encapsulates a data acquisition thread for
//   vAmp devices
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
#ifndef VAMP_THREAD_H
#define VAMP_THREAD_H

#if __BORLANDC__
# include <vcl.h>
#else // __BORLANDC__
# include <windows.h>
# include <QPixmap>
#endif // __BORLANDC__

#include <vector>
#include <string>
#include <map>
#include <set>
#include "GenericSignal.h"
#include "IIRFilter.h"
#include "Color.h"
#include "defines.h"
#include "OSThread.h"
#include "FirstAmp.h"
#include "vAmpChannelInfo.h"

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
    int devID,
    int mode,
    float hpCorner
  );
  virtual ~vAmpThread();

  float ExtractData(int ch, int sample);
  std::vector<float> GetImpedances();
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

    int   mBlockSize,
          mTimeout,
          mBufSize,
          mNumPoints,
          mWriteCursor,
          mReadCursor,
          mRingBufferSize,
          mDecimate,
          mNumChannels,
          mImpInitState; // contains the feedback value from Impedance initialisation
    float mSampleRate;
    unsigned short mPrevTime;
    std::ostringstream mLastErr,
                       mWarnings;
    std::vector<int> mChList;
    int mMode;
    int mDevChs;
    GenericSignal mDataBuffer, mDataOutput;

    std::vector<float> mImpArray;
    std::valarray<float> mTrigBuffer; //mdatabuffer[device][ch][sample]
    float *mBuffer;

    HANDLE mEvent,
           mDev;
    bool mOk;
    bool mIs8Channel;
    unsigned int mNumDevices;
    int mDevID;
    int m_nChannelMode;
    int m_nEEGChannels;
    int m_nAUXChannels;
    bool m_bOpen;
    unsigned int mDataCounterErrors;
    int mStartMode;
    int mBufferSize;
    int m_nMaxPoints;
    t_faInformation m_DeviceInfo;      // Device info.
    t_faProperty    m_DeviceProp;      // Channel properties.
    std::vector<CChannelInfo> m_tblChanInfo;

    t_faDataFormatMode20kHz* m_tblMaxBuf4;     // 1 read cycle buffer (highspeed, 4 ch) + 2 add. samples.
    t_faDataModel8*          m_tblMaxBuf8;     // 1 read cycle buffer of 8 channel system + 2 add. samples.
    t_faDataModel16*         m_tblMaxBuf16;    // 1 read cycle buffer of 16 channel system + 2 add. samples.
    std::vector<float>   m_tblEEGData;      // 1 read cycle of only EEG and AUX signals.
    std::vector<float>   m_tblTrigger;      // 1 read cycle of only Trigger signals.
    std::vector<float>   m_tblPacket;       // 1 read cycle of EEG, AUX, Trigger signals.
    t_faDataModeSettings mFastSettings;
    t_faDataMode mDataMode;
    bool mHighSpeed;
    std::set<int> mDigChs;

    int ReadData(int nDeviceId, char *pBuffer, int nReadLen);
    IIRFilter<float> mFilter;
    typedef Ratpoly<FilterDesign::Complex> TransferFunction;
};

#endif // VAMP_THREAD_H
//---------------------------------------------------------------------------

