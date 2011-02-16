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
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIError.h"
#include "PrecisionTime.h"
#include "vAmpThread.h"
#include "vAmpDisplay.h"
#include "FirstAmp.h"

using namespace std;


vAmpThread::vAmpThread(
  int inBlockSize,
  float sampleRate,
  int decimate,
  const vector<int>& chList,
  int devID,
  int mode,
  float hpCorner
)
: mBlockSize( inBlockSize ),
  mSampleRate(sampleRate),
  mWriteCursor( 0 ),
  mReadCursor( 0 ),
  mBuffer(NULL),
  mDecimate(decimate),
  acquireEventRead(NULL),
  mNumDevices(0),
  mDevID(devID),
  mChList(chList),
  mMode(mode),
  mEvent( NULL ),
  m_nMaxPoints( 0 ),
  mLock( false ),
  m_tblMaxBuf4( NULL ),
  m_tblMaxBuf8( NULL ),
  m_tblMaxBuf16( NULL ),
  mDataCounterErrors( 0 )
{
  mOk = true;
  mNumDevices = faGetCount();
  mLastErr.str("");
  mHighSpeed = mMode == 1 || mMode == 4;
  if (mNumDevices < 1) {
    mLastErr <<"No vAmp devices were found."<<endl;
    mOk =false;
    return;
  }
  if (mNumDevices > 1) {
    mLastErr << "A maximum of 1 devices can be present on the system at a time."<<endl;
    mOk =false;
    return;
  }

  //open the devices
  mNumChannels = 0;
  mRingBufferSize = int(mBlockSize*cBlocksInRingBuffer);
  mTrigBuffer.resize( 0 );

  m_nMaxPoints = mBlockSize*decimate;
  if (mMode == 3){
    m_nMaxPoints = 1;
    mImpArray.clear();
  }

  if (faOpen(mDevID) != FA_ERR_OK)
  {
    mLastErr << "Error opening device " << mDevID << endl;
    mOk =false;
    return;
  }
  memset(&m_DeviceInfo, 0, sizeof(m_DeviceInfo));

  mDigChs.clear();
  if (faGetInformation(mDevID, &m_DeviceInfo) != FA_ERR_OK)
  {
    mLastErr << "Failed getting device information for " << mDevID <<endl;
    mOk =false;
    return;
  }
  switch (m_DeviceInfo.Model)
  {
        case FA_MODEL_8:
            m_nChannelMode = DEVICE_CHANMODE_8;
            m_nEEGChannels = FA_MODEL_8_CHANNELS_MAIN - 1; // without trigger
            m_nAUXChannels = FA_MODEL_8_CHANNELS_AUX;
            delete[] m_tblMaxBuf8;
            m_tblMaxBuf8 = new t_faDataModel8[m_nMaxPoints];
            m_tblChanInfo.resize(m_nEEGChannels + m_nAUXChannels + 1); // 1 Trigger.
            if(faSetDataMode(mDevID, dmNormal, NULL)!=FA_ERR_OK)
            {
              mLastErr << "Error setting data mode." << endl;
              mOk = false;
              return;
            }
            mIs8Channel = true;
            break;
        case FA_MODEL_16:
            m_nChannelMode = DEVICE_CHANMODE_16;
            delete[] m_tblMaxBuf16;
            m_tblMaxBuf16 = new t_faDataModel16[m_nMaxPoints];
            m_nEEGChannels = FA_MODEL_16_CHANNELS_MAIN - 1; // without trigger
            m_nAUXChannels = FA_MODEL_16_CHANNELS_AUX;
            m_tblChanInfo.resize(m_nEEGChannels + m_nAUXChannels + 1); // 1 Trigger.
            if(faSetDataMode(mDevID, dmNormal, NULL)!=FA_ERR_OK)
            {
              mLastErr << "Error setting data mode." << endl;
              mOk = false;
              return;
            }
            mIs8Channel = false;
            break;
        default: // Unknow device (error).
            mLastErr << "Unknown device model for device " << mDevID << endl;
            mOk =false;
          return;
  }

  if (mHighSpeed)
  {
        m_nChannelMode = DEVICE_CHANMODE_4;
        delete[] m_tblMaxBuf4;
        m_tblMaxBuf4 = new t_faDataFormatMode20kHz[m_nMaxPoints];
        m_nEEGChannels = FA_MODE_20_KHZ_CHANNELS_MAIN; // EEG
        m_nAUXChannels = 0; // no AUX.
        m_tblChanInfo.resize(m_nEEGChannels + m_nAUXChannels + 1); // 1 Trigger.
        if(faSetDataMode(mDevID, dm20kHz4Channels, &mFastSettings)!=FA_ERR_OK)
        {
          mLastErr << "Error setting data mode." << endl;
          mOk = false;
          return;
        }
  }
  for (size_t ch = 0; ch < mChList.size(); ch++)
  {
    if (mMode == 1)
    {
        mFastSettings.Mode20kHz4Channels.ChannelsPos[mChList[ch]] = mChList[ch];
        mFastSettings.Mode20kHz4Channels.ChannelsNeg[mChList[ch]] = -1;
    }
    if (mChList[ch] == m_nEEGChannels + m_nAUXChannels)
        mDigChs.insert(ch);
  }

  // Retrieves device properties.
  memset(&m_DeviceProp, 0, sizeof(m_DeviceProp));
  if (faGetProperty(mDevID, &m_DeviceProp) != FA_ERR_OK)
  {
    mLastErr << "Error getting device properties for device " << mDevID << endl;
    mOk =false;
    return;
  }

  // Channel type, channel resolution.
  for (UINT i = 0; i < m_tblChanInfo.size(); i++)
  {
      CChannelInfo& ci = m_tblChanInfo[i];
      if (i < UINT(m_nEEGChannels)) // EEG
      {
          ci.nType = DEVICE_CHAN_TYPE_EEG;
          ci.dResolution = double(m_DeviceProp.ResolutionEeg * 1e6); // V > µV
      }
      else if (m_nAUXChannels > 0 && // AUX present
          i >= UINT(m_nEEGChannels) && i < m_tblChanInfo.size() -1 ) // AUX
      {
          ci.nType = DEVICE_CHAN_TYPE_AUX;
          ci.dResolution = double(m_DeviceProp.ResolutionAux * 1e6); // V > µV
      }
      else // Digital port.
      {
          ci.nType = DEVICE_CHAN_TYPE_TRIGGER;
          ci.dResolution = 1.0f;
      }
  }
  m_tblEEGData.resize((m_nEEGChannels + m_nAUXChannels)*mBlockSize*decimate);
  m_tblTrigger.resize(mBlockSize);
  m_tblPacket.resize((m_nEEGChannels + m_nAUXChannels+1)*mBlockSize*decimate); // actual results.


  mDataBuffer.SetProperties(SignalProperties(mChList.size(), mBlockSize*decimate, SignalType::float32));
  mDataOutput = mDataBuffer;
  mBufSize = mChList.size()*mRingBufferSize;
  mBuffer = new float[mBufSize];

  TransferFunction tf(1.0);
  float outGain = 1.0;
  TransferFunction lp =
        FilterDesign::Butterworth().Order( 2 ).Lowpass(.45/mDecimate).TransferFunction();
  outGain /= abs( lp.Evaluate( 1.0 ) );
  tf *= lp;

  if (hpCorner > 0)
  {
    TransferFunction hp  =
          FilterDesign::Butterworth().Order( 2 ).Highpass(hpCorner/mDecimate).TransferFunction();
    outGain /= abs(hp.Evaluate(-1.0));
    tf *= hp;
  }

  mFilter.SetZeros(tf.Numerator().Roots())
         .SetPoles(tf.Denominator().Roots())
         .SetGain(outGain)
         .Initialize(mDataBuffer.Channels());

  acquireEventRead = CreateEvent(NULL, true, false, "ReadEvent");

  if( mMode != 3 )
    vAmpDisplay::DisplayLogo( mDevID );
  t_faDataState state = { sizeof( t_faDataState ), };
  faGetDataState( mDevID, &state );
  mDataCounterErrors = state.DataCounterErrors;
  // faStart/faStop etc must be called from the main thread.
  if( faStart(mDevID) != FA_ERR_OK )
  {
    mLastErr << "Error starting vAmp device." << endl;
    mOk = false;
    return;
  }
  switch (mMode)
  {
    case 0:
    case 1:
        break;
    case 2:
    case 4:
        if(faStartCalibration(mDevID)!=FA_ERR_OK)
        {
          mLastErr << "Error starting calibration." << endl;
          mOk = false;
          return;
        }
        break;
    case 3:
        if(faStartImpedance(mDevID)!=FA_ERR_OK)
        {
          mLastErr << "Error starting impedance measurement." << endl;
          mOk = false;
          return;
        }
        break;
  }
}

vAmpThread::~vAmpThread()
{
  delete[] mBuffer;
  CloseHandle(acquireEventRead);
  // faStart/faStop etc must be called from the main thread.
  vAmpDisplay::Clear( mDevID );
  switch (mMode){
      case 0:
      case 1:
          break;
      case 2:
      case 4:
          faStopCalibration(mDevID);
          break;
      case 3:
          faStopImpedance(mDevID);
          break;
  }
  faStop(mDevID);
  faClose(mDevID);

  delete[] m_tblMaxBuf4;
  delete[] m_tblMaxBuf8;
  delete[] m_tblMaxBuf16;
}

vector<float>
vAmpThread::GetImpedances()
{
  Lock();
  vector<float> impedances = mImpArray;
  Unlock();
  return impedances;
}

string
vAmpThread::GetLastWarning()
{
  Lock();
  string s = mWarnings.str();
  mWarnings.str( "" );
  Unlock();
  return s;
}

void vAmpThread::AdvanceReadBlock()
{
  mReadCursor += (mChList.size()*mBlockSize);
  mReadCursor %= mBufSize;
}

float
vAmpThread::ExtractData(int ch, int sample)
{
  return mBuffer[mReadCursor + sample*mChList.size() + ch];
/*
  while( mReadCursor == mWriteCursor && mEvent != NULL )
    ::WaitForSingleObject( mEvent, mTimeout );
  sint16 value = *reinterpret_cast<sint16*>( mpBuffer + mReadCursor );
  mReadCursor += sizeof( sint16 );
  mReadCursor %= mBufSize;
  return value;    */
}

int
vAmpThread::Execute()
{
    char *pMaxBuffer = NULL;
    mOk = true;
    int returnLen = 0,
        nReadLen = 0;
    mWriteCursor = mReadCursor = 0;
    int readPos = 0;

    memset(mBuffer, 0, mBufSize);

    int curSample = 0;
    int waitTime;
    mPrevTime = PrecisionTime::Now();
    PrecisionTime tLastDisplayUpdate = PrecisionTime::Now();
    bool bIsStart = true;

    while (!this->IsTerminating() && mOk)
    {
        unsigned short tnow = PrecisionTime::Now();
        if (mMode == 3) 
        {    
           // GET IMPEDANCE AND CONTINUE            
           mImpArray.clear(); 
           if(mIs8Channel) 
           { 
               unsigned int pBuf[FA_MODEL_16_CHANNELS_MAIN]; // faGetImpedance() assumes a dimension of FA_MODEL_16_CHANNELS_MAIN
               for (int i = 0; i < FA_MODEL_8_CHANNELS_MAIN; i++) {pBuf[i] = 0;} 
               if(faGetImpedance(mDevID, pBuf, sizeof(pBuf))==FA_ERR_OK) 
               { 
                  for (int i = 0; i < FA_MODEL_8_CHANNELS_MAIN; i++) 
                  { 
                       mImpArray.push_back(float(pBuf[i])); 
                  } 
               } 
               else 
               { // Restart the serving routing, reset the device. 
                  mOk = false; 
                  mLastErr << "Error reading impedances. Restart." <<endl; 
                  return -1; 
               } 
           } 
           else 
           { 
               unsigned int pBuf[FA_MODEL_16_CHANNELS_MAIN]; 
               for (int i = 0; i < FA_MODEL_16_CHANNELS_MAIN; i++) {pBuf[i] = 0;} 
               if(faGetImpedance(mDevID, pBuf, sizeof(pBuf))==FA_ERR_OK) 
               { 
                  for (int i = 0; i < FA_MODEL_16_CHANNELS_MAIN; i++) 
                  { 
                       mImpArray.push_back(float(pBuf[i])); 
                  } 
               } 
               else 
               { // Restart the serving routing, reset the device. 
                  mOk = false; 
                  mLastErr << "Error reading impedances. Restart." <<endl; 
                  return -1; 
               } 
           }    
           //This is done to prevent the display from being updated to often...paying attention to performance issues described in vAmp SDK. 
           if(bIsStart||(PrecisionTime::TimeDiff(tLastDisplayUpdate , PrecisionTime::Now())>=cDisplayUpdateTime)) 
           { 
                tLastDisplayUpdate = PrecisionTime::Now(); 
                bIsStart = false; 
                if(vAmpDisplay::DisplayImpedances( mDevID, mImpArray )!=FA_ERR_OK) 
                { 
                     mOk = false; 
                     mLastErr << "Error displaying impedances. Restart." <<endl; 
                     return -1; 
                }                                  
           }             
           waitTime = min(mBlockSize/mSampleRate*1000 - PrecisionTime::TimeDiff(tnow, PrecisionTime::Now()),1000*mBlockSize/mSampleRate);
           if (waitTime > 0) Sleep(waitTime);
           SetEvent( acquireEventRead );
           continue;
        }
        //ACQUIRE DATA
        switch (m_nChannelMode)
        {
                case DEVICE_CHANMODE_16:
                    pMaxBuffer = (char *)&m_tblMaxBuf16;
                    nReadLen = m_nMaxPoints * sizeof(t_faDataModel16); // in bytes.
                    returnLen = ReadData(mDevID, pMaxBuffer, nReadLen);
                    if (returnLen < 0) // Device broken. Restart device.
                    {
                        // Restart the serving routing, reset the device.
                        mOk = false;
                        mLastErr << "Error reading data. Restart." <<endl;
                        return -1;
                    }
                    if (returnLen != nReadLen) // Device broken. Restart device.
                    {
                        mOk = false;
                        mLastErr << "Error reading data. Restart. ("<<returnLen<<","<<nReadLen<<")" <<endl;
                        return -1;
                    }
                    // Serving data successfully.
                    // Copy data to ring buffer.
                    if (returnLen >= nReadLen)
                    {
                        for (int sample = 0; sample < mBlockSize*mDecimate; sample++)
                        {
                          for (int chPos = 0; chPos < (int)mChList.size(); chPos++) 
                          { 
                             int nPos = mChList[chPos];//the channel of the device that should appear in this position according to parameter definition 
                             // Process input of normal data channels (subtract reference if not in calibration mode)                             
                             if(0<=nPos&& nPos<16) 
                             { 
                               mDataBuffer(chPos,sample) = 
                                  (m_tblMaxBuf16[sample].Main[nPos] - ((mMode == 2) ? 0 : m_tblMaxBuf16[sample].Main[16]))*m_tblChanInfo[nPos].dResolution; 
                                           
                             } 
                             //auxiliary channel 1 
                             if(nPos ==16) mDataBuffer(chPos,sample) = m_tblMaxBuf16[sample].Aux[0]*m_tblChanInfo[16].dResolution; 
                             //auxiliary channel 2 
                             if(nPos ==17) mDataBuffer(chPos,sample) = m_tblMaxBuf16[sample].Aux[1]*m_tblChanInfo[16].dResolution; 
                             //Trigger Channel 
                             if(nPos==18)  mDataBuffer(chPos,sample) = m_tblMaxBuf16[sample].Status & 0x1ff;
                          } 
                       } 
                    } 
                    break;
                case DEVICE_CHANMODE_4:
                    pMaxBuffer = (char *)&m_tblMaxBuf4[0];
                    nReadLen = m_nMaxPoints * sizeof(t_faDataFormatMode20kHz); // in bytes.
                    returnLen = ReadData(mDevID, pMaxBuffer, nReadLen);
                    if (returnLen < 0) // Device broken. Restart device.
                    {
                        // Restart the serving routing, reset the device.
                        mOk = false;
                        mLastErr << "Error reading data. Restart." <<endl;
                        return -1;
                    }
                    if (returnLen != nReadLen) // Device broken. Restart device.
                    {
                        mOk = false;
                        mLastErr << "Error reading data. Restart. ("<<returnLen<<","<<nReadLen<<")" <<endl;
                        return -1;
                    }
                    // Serving data successfully.
                    // Copy data to ring buffer.
                    if (returnLen >= nReadLen)
                    {
                      for (int sample = 0; sample < mBlockSize*mDecimate; sample++)
                      {
                        for (int chPos = 0; chPos < (int)mChList.size(); chPos++) 
                        { 
                            int nPos = mChList[chPos]; 
                            if(0<=nPos&&nPos<4)//data channels 
                            { 
                                mDataBuffer(chPos,sample) = (m_tblMaxBuf4[sample].Main[nPos])*m_tblChanInfo[nPos].dResolution; 
                            } 
                            //trigger channel 
                            if(nPos ==4)mDataBuffer(chPos,sample) = m_tblMaxBuf4[sample].Status & 0x1ff;
                        }
                      }
                    }
                    break;
                default:
                    pMaxBuffer = (char *)&m_tblMaxBuf8[0];
                    nReadLen = m_nMaxPoints * sizeof(t_faDataModel8); // in bytes.
                    returnLen = ReadData(mDevID, pMaxBuffer, nReadLen);
                    if (returnLen < 0) // Device broken. Restart device.
                    {
                        // Restart the serving routing, reset the device.
                        mOk = false;
                        mLastErr << "Error reading data. Restart." <<endl;
                        return -1;
                    }
                    if (returnLen != nReadLen) // Device broken. Restart device.
                    {
                        mOk = false;
                        mLastErr << "Error reading data. Restart. ("<<returnLen<<","<<nReadLen<<")" <<endl;
                        return -1;
                    }
                    // Serving data successfully.
                    // Copy data to ring buffer.
                    if (returnLen >= nReadLen)
                    {
                        for (int sample = 0; sample < mBlockSize*mDecimate; sample++)
                        {
                          for (int chPos = 0; chPos < (int)mChList.size(); chPos++) 
                          { 
                            int nPos = mChList[chPos];                            
                            //data channels 
                            if(0<=nPos&& nPos<8) 
                            {                                      
                                mDataBuffer(chPos,sample) = 
                                    (m_tblMaxBuf8[sample].Main[nPos] - ((mMode == 2) ? 0 : m_tblMaxBuf8[sample].Main[8]))*m_tblChanInfo[nPos].dResolution;
                            }
                            // Auxiliary channel 1 
                            if(nPos ==8)mDataBuffer(chPos,sample) = m_tblMaxBuf8[sample].Aux[0]*m_tblChanInfo[8].dResolution; 
                            // Auxiliary channel 2 
                            if(nPos ==9)mDataBuffer(chPos,sample) = m_tblMaxBuf8[sample].Aux[1]*m_tblChanInfo[8].dResolution; 
                            //Trigger channel 
                            if(nPos ==10)mDataBuffer(chPos,sample) = m_tblMaxBuf8[sample].Status & 0x1ff;
                          }
                        }
                    }
                    break;
        }
        t_faDataState state = { sizeof( t_faDataState ), };
        faGetDataState( mDevID, &state );
        if( state.DataCounterErrors != mDataCounterErrors )
        {
            Lock();
            mWarnings << "Amplifier reports data loss. "
                      << "The total number of counter errors is "
                      << state.DataCounterErrors
                      << endl;
            Unlock();
            mDataCounterErrors = state.DataCounterErrors;
        }
        //FILTER
        mFilter.Process(mDataBuffer, mDataOutput);

        //DECIMATE
        Lock();
        for (int sample = 0; sample < mDataOutput.Elements(); sample+=mDecimate)
        {
            for (int ch = 0; ch < mDataOutput.Channels(); ch++)
            {
                if (mDigChs.find(ch)!= mDigChs.end())
                    mBuffer[mWriteCursor++] = mDataBuffer(ch, sample);
                else
                    mBuffer[mWriteCursor++] = mDataOutput(ch, sample);
                mWriteCursor %= mBufSize;
            }
        }
        Unlock();
        SetEvent( acquireEventRead );
    }
    return 0;
}

int vAmpThread::ReadData(int nDeviceId, char *pBuffer, int nReadLen)
{
    int nReturnLen = 0,			// Current return length in bytes.
        nLenToRead = nReadLen;	// len in bytes.
    int nLoops = 0;
#if 0
    unsigned short readTime, startTime = PrecisionTime::Now();
    int tdiff, remTime;
    DWORD sleepTime;
#endif
    do
    {
        //wait until at least one block has passed

        /*if (nLoops == 0){
            tdiff = PrecisionTime::TimeDiff(mPrevTime, PrecisionTime::Now());
            remTime = (int)((float(mBlockSize)*1000)/(mSampleRate)) - (tdiff);
            if (remTime >0 && remTime < (float(mBlockSize)*1000)/(mSampleRate) )
                Sleep(remTime);
        } */

        nReturnLen = faGetData(nDeviceId, pBuffer, nLenToRead);
        nLenToRead -= nReturnLen;
#if 0
        readTime = PrecisionTime::TimeDiff(startTime, PrecisionTime::Now());
        if (readTime >(int)((float(2*mBlockSize)*1000)/(mSampleRate)) && nLenToRead > 0)
            return -1;
#endif
        //fprintf(logFile,"%d ", readTime);
        if (nReturnLen < 0) // Device error.
        {
            return -1;
        }

        if (nReturnLen == 0){
            //fprintf(logFile,"(LEN=0)");
            Sleep(3);
            continue;
        }
        if (nLenToRead > 0)
        {
             // decrement the read length.
            pBuffer += nReturnLen;
#if 0
            tdiff = PrecisionTime::TimeDiff(mPrevTime, PrecisionTime::Now());
            remTime = (int)((float(mBlockSize)*1000)/(mSampleRate)) - (tdiff);
            if (remTime >0 && remTime < (float(mBlockSize)*1000)/(mSampleRate)) {
                sleepTime = remTime;
                Sleep(0);
                //fprintf(logFile,"(S%d) ", remTime);
            }
#endif
        }
        //nLoops++;
        /*if (PrecisionTime::TimeDiff(nTimeStart, PrecisionTime::Now()) >= DEVICE_SERVE_TIMEOUT) // Timeout
        {
            return -1;
        }  */
    } while (nLenToRead > 0 && !this->IsTerminating());
    //fprintf(logFile,"\n\n");
    mPrevTime = PrecisionTime::Now();
    return (nLenToRead <= 0) ? nReadLen : (nReadLen - nLenToRead);

    //original FirstAmp code
    /*
    int nReturnLen = 0,			// Current return length in bytes.
        nLenToRead = nReadLen;	// len in bytes.
    UINT nTimeOut = timeGetTime() + DEVICE_SERVE_TIMEOUT;

    do
    {
        nReturnLen = faGetData(nDeviceId, pBuffer, nLenToRead);
        if (nReturnLen < 0) // Device error.
        {
            return -1;
        }
        else if (nReturnLen == 0)
        {
            Sleep(DEVICE_GET_DATA_INTERVAL); // previous 1 ms: causes trigger problem.
        }
        else
        {
            nLenToRead -= nReturnLen; // decrement the read length.
            pBuffer += nReturnLen;
        }
        if (timeGetTime() >= nTimeOut) // Timeout
        {
            return -1;
        }
    } while (m_bRun && nLenToRead > 0);
    return (nLenToRead <= 0) ? nReadLen : (nReadLen - nLenToRead);
    */
}

