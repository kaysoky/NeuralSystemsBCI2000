////////////////////////////////////////////////////////////////////////////////
// $Id: vAmpThread.h 2032 2008-06-26 17:11:32Z mellinger $
// Author: jadamwilson2@gmail.com
// Description: A class that encapsulates a data acquisition thread for
//   vAmp devices
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIError.h"
#include "PrecisionTime.h"
#include "GUI.h"
#include "images/BCI2000logo_small.h"
#include "vAmpThread.h"
#include "FirstAmp.h"

using namespace std;
using namespace GUI;

vAmpThread::vAmpThread(
  int inBlockSize,
  float sampleRate,
  int decimate,
  const vector<int>& chList,
  int chsPerDev[MAX_ALLOWED_DEVICES],
  const vector<int>& devList,
  int mode,
  float hpCorner
)
: OSThread( true ),
  mBlockSize( inBlockSize ),
  mSampleRate(sampleRate),
  mWriteCursor( 0 ),
  mReadCursor( 0 ),
  mBuffer(NULL),
  mDecimate(decimate),
  acquireEventRead(NULL),
  mNumDevices(0),
  mDevList(devList),
  mChList(chList),
  mMode(mode),
  mEvent( NULL ),
  m_nMaxPoints( 0 ),
  mLock( false )
{
  for( int i = 0; i < MAX_ALLOWED_DEVICES; ++i )
  {
    m_tblMaxBuf4[ i ] = NULL;
    m_tblMaxBuf8[ i ] = NULL;
    m_tblMaxBuf16[ i ] = NULL;
    mDataCounterErrors[ i ] = 0;
  }

  mOk = true;
  mNumDevices = faGetCount();
  mLastErr.str("");
  mHighSpeed = mMode == 1 || mMode == 4;
  if (mNumDevices < 1) {
    mLastErr <<"No vAmp devices were found."<<endl;
    mOk =false;
    return;
  }
  if (mNumDevices > MAX_ALLOWED_DEVICES) {
    mLastErr << "A maximum of " << MAX_ALLOWED_DEVICES << " devices can be present on the system at a time."<<endl;
    mOk =false;
    return;
  }

  //open the devices
  mNumChannels = 0;
  mRingBufferSize = int(mBlockSize*cBlocksInRingBuffer);
  mTrigBuffer.resize(MAX_ALLOWED_DEVICES);

  m_nMaxPoints = mBlockSize*decimate;
  if (mMode == 3){
    m_nMaxPoints = 1;
    mImpArray.resize(mNumDevices);
  }
  for (size_t dev = 0; dev < mDevList.size(); dev++)
  {
    if (faOpen(mDevList[dev]) != FA_ERR_OK){
        mLastErr << "Error opening device " << mDevList[dev] << endl;
        mOk =false;
      return;
    }
    memset(&m_DeviceInfo[dev], 0, sizeof(m_DeviceInfo[dev]));
    mChsPerDev[dev] = chsPerDev[dev];
    mDevChMap[dev].clear();
    mDevChRevMap[dev].clear();
    mDigChs.clear();
    if (faGetInformation(mDevList[dev], &(m_DeviceInfo[dev])) != FA_ERR_OK)
    {
        mLastErr << "Failed getting device information for " << mDevList[dev]<<endl;
        mOk =false;
      return;
    }
    switch (m_DeviceInfo[dev].Model)
    {
        case FA_MODEL_8:
            m_nChannelMode[dev] = DEVICE_CHANMODE_8;
            m_nEEGChannels[dev] = FA_MODEL_8_CHANNELS_MAIN - 1; // without trigger
            m_nAUXChannels[dev] = FA_MODEL_8_CHANNELS_AUX;
            delete[] m_tblMaxBuf8[dev];
            m_tblMaxBuf8[dev] = new t_faDataModel8[m_nMaxPoints];
            m_tblChanInfo[dev].resize(m_nEEGChannels[dev] + m_nAUXChannels[dev] + 1); // 1 Trigger.
            faSetDataMode(mDevList[dev], dmNormal, NULL);
            break;
        case FA_MODEL_16:
            m_nChannelMode[dev] = DEVICE_CHANMODE_16;
            delete[] m_tblMaxBuf16[dev];
            m_tblMaxBuf16[dev] = new t_faDataModel16[m_nMaxPoints];
            m_nEEGChannels[dev] = FA_MODEL_16_CHANNELS_MAIN - 1; // without trigger
            m_nAUXChannels[dev] = FA_MODEL_16_CHANNELS_AUX;
            m_tblChanInfo[dev].resize(m_nEEGChannels[dev] + m_nAUXChannels[dev] + 1); // 1 Trigger.
            faSetDataMode(mDevList[dev], dmNormal, NULL);
            break;
        default: // Unknow device (error).
            mLastErr << "Unknown device model for device " << mDevList[dev] << endl;
            mOk =false;
          return;
    }

    if (mHighSpeed)
    {
        m_nChannelMode[dev] = DEVICE_CHANMODE_4;
        delete[] m_tblMaxBuf4[dev];
        m_tblMaxBuf4[dev] = new t_faDataFormatMode20kHz[m_nMaxPoints];
        m_nEEGChannels[dev] = FA_MODE_20_KHZ_CHANNELS_MAIN; // EEG
        m_nAUXChannels[dev] = 0;	// no AUX.
        m_tblChanInfo[dev].resize(m_nEEGChannels[dev] + m_nAUXChannels[dev] + 1); // 1 Trigger.
        faSetDataMode(mDevList[dev], dm20kHz4Channels, mFastSettings);
    }
    for (size_t ch = 0; ch < mChList.size(); ch++)
    {
        if (mChList[ch] >= mNumChannels && mChList[ch] < (mNumChannels + m_nEEGChannels[dev] + m_nAUXChannels[dev]+1))
        {
            mDevChMap[dev][mChList[ch]] = mChList[ch] - mNumChannels;
            mDevChRevMap[dev][mChList[ch] - mNumChannels] = ch;
            if (mMode == 1){
                mFastSettings[dev].Mode20kHz4Channels.ChannelsPos[int(mChList[ch] - mNumChannels)] = int(mChList[ch] - mNumChannels);
                mFastSettings[dev].Mode20kHz4Channels.ChannelsNeg[int(mChList[ch] - mNumChannels)] = -1;
            }
            if (mChList[ch] == mNumChannels + m_nEEGChannels[dev] + m_nAUXChannels[dev])
                mDigChs.insert(ch);
        }
    }

    //mDevMap[dev].clear();


    // Retrieves device properties.
    memset(&m_DeviceProp[dev], 0, sizeof(m_DeviceProp[dev]));
    if (faGetProperty(mDevList[dev], &m_DeviceProp[dev]) != FA_ERR_OK)
    {
        mLastErr << "Error getting device properties for device " << mDevList[dev]<<endl;
        mOk =false;
      return;
    }

    // Channel type, channel resolution.
    for (UINT i = 0; i < m_tblChanInfo[dev].size(); i++)
    {
        CChannelInfo& ci = m_tblChanInfo[dev][i];
        if (i < UINT(m_nEEGChannels[dev])) // EEG
        {
            ci.nType = DEVICE_CHAN_TYPE_EEG;
            ci.dResolution = double(m_DeviceProp[dev].ResolutionEeg * 1e6); // V > µV
        }
        else if (m_nAUXChannels[dev] > 0 && // AUX present
            i >= UINT(m_nEEGChannels[dev]) && i < m_tblChanInfo[dev].size() -1 ) // AUX
        {
            ci.nType = DEVICE_CHAN_TYPE_AUX;
            ci.dResolution = double(m_DeviceProp[dev].ResolutionAux * 1e6); // V > µV
        }
        else // Digital port.
        {
            ci.nType = DEVICE_CHAN_TYPE_TRIGGER;
            ci.dResolution = 1.0f;
        }
    }
    m_tblEEGData[dev].resize((m_nEEGChannels[dev] + m_nAUXChannels[dev])*mBlockSize*decimate);
    m_tblTrigger[dev].resize(mBlockSize);
    m_tblPacket[dev].resize((m_nEEGChannels[dev] + m_nAUXChannels[dev]+1)*mBlockSize*decimate); // actual results.
    mNumChannels += mChsPerDev[dev];
  }

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

  if (hpCorner > 0){
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

  for (size_t dev = 0; dev < mNumDevices; dev++){
      DisplayBCI2000Logo( mDevList[dev] );
      t_faDataState state = { sizeof( t_faDataState ), };
      faGetDataState( mDevList[dev], &state );
      mDataCounterErrors[dev] = state.DataCounterErrors;
      // faStart/faStop etc must be called from the main thread.
      faStart(mDevList[dev]);
      switch (mMode){
          case 0:
          case 1:
              break;
          case 2:
          case 4:
              faStartCalibration(mDevList[dev]);
              break;
          case 3:
              faStartImpedance(mDevList[dev]);
              break;
      }
  }
}

vAmpThread::~vAmpThread()
{
  delete[] mBuffer;
  CloseHandle(acquireEventRead);
  // faStart/faStop etc must be called from the main thread.
  for (size_t dev = 0; dev < mNumDevices; dev++){
      ClearAmpDisplay( mDevList[dev] );
      switch (mMode){
          case 0:
          case 1:
              break;
          case 2:
          case 4:
              faStopCalibration(mDevList[dev]);
              break;
          case 3:
              faStopImpedance(mDevList[dev]);
              break;
      }
      faStop(mDevList[dev]);
      faClose(mDevList[dev]);
  }

  for( int dev = 0; dev < MAX_ALLOWED_DEVICES; ++dev )
  {
    delete[] m_tblMaxBuf4[dev];
    delete[] m_tblMaxBuf8[dev];
    delete[] m_tblMaxBuf16[dev];
  }
}

vector< vector<float> >
vAmpThread::GetImpedances()
{
  Lock();
  vector< vector<float> > impedances = mImpArray;
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


void
vAmpThread::DisplayBCI2000Logo( int inID )
{
    Graphics::TBitmap* pBitmap = NewBitmap();
    int logoWidth = GraphicResource::Width( Resources::BCI2000logo_small ),
        logoHeight = GraphicResource::Height( Resources::BCI2000logo_small );
    pBitmap->Canvas->Brush->Color = clWhite;
    pBitmap->Canvas->FillRect( TRect( 0, 0, pBitmap->Width, pBitmap->Height ) );
    int left = ( pBitmap->Width - logoWidth ) / 2,
        top = ( pBitmap->Height - logoHeight ) / 2;
    DrawContext dc = { pBitmap->Canvas->Handle, { left, top, logoWidth, logoHeight } };
    GraphicResource::Render<RenderingMode::Transparent>( Resources::BCI2000logo_small, dc );
    faSetBitmap( inID, pBitmap->Handle );
    delete pBitmap;

    faSetContrast( inID, 100 );
    faSetBrightness( inID, 100 );
}

void vAmpThread::ClearAmpDisplay( int inID )
{
    Graphics::TBitmap* pBitmap = NewBitmap();
    pBitmap->Canvas->Brush->Color = clBlack;
    pBitmap->Canvas->FillRect( TRect( 0, 0, pBitmap->Width, pBitmap->Height ) );
    faSetBitmap( inID, pBitmap->Handle );
    delete pBitmap;
}

void vAmpThread::DisplayImpedances( int inID, const vector<float>& inImpedances )
{ // Draw a two-column table for channel impedances, and a wide field for
  // the reference's impedance below it.
  Graphics::TBitmap* pBitmap = NewBitmap();
  float deltaX = pBitmap->Width / 2.0,
        deltaY = pBitmap->Height / 9.0;
  TRect rects[17]; // 2 * 8 rects for channels, 1 rect for reference
  for( int i = 0; i < 8; ++i )
  {
      rects[i].left = 0;
      rects[i].right = deltaX;
      rects[i + 8].left = deltaX;
      rects[i + 8].right = pBitmap->Width;
      rects[i].top = i * deltaY;
      rects[i].bottom = ( i + 1 ) * deltaY;
      rects[i + 8].top = rects[i].top;
      rects[i + 8].bottom = rects[i].bottom;
  }
  rects[16].left = 0;
  rects[16].right = pBitmap->Width;
  rects[16].top = rects[15].bottom;
  rects[16].bottom = pBitmap->Height;
  const int numRects = sizeof( rects ) / sizeof( *rects );

  // Background
  pBitmap->Canvas->Brush->Color = clWhite;
  pBitmap->Canvas->FillRect( TRect( 0, 0, pBitmap->Width, pBitmap->Height ) );
  // Fields
  const int frame = 1;
  int idx[numRects];
  string labels[numRects];
  for( size_t i = 0; i < inImpedances.size() - 1; ++i )
  {
    idx[i] = i;
    ostringstream oss;
    oss << "Ch " << i + 1 << ": ";
    labels[i] = oss.str();
  }
  for( int i = inImpedances.size() - 1; i < numRects - 1; ++i )
    idx[i] = -1;
  idx[numRects - 1] = inImpedances.size() - 1;
  labels[numRects - 1] = "Ref: ";

  pBitmap->Canvas->Font->Color = clWhite;
  pBitmap->Canvas->Font->Height = -16;
  pBitmap->Canvas->Font->Style = ( TFontStyles() << fsBold );
  TSize size = pBitmap->Canvas->TextExtent( "w" );

  for( int i = 0; i < numRects; ++i )
  {
    TColor c;
    string s;
    if( idx[i] >= 0 )
    {
      ValueToText( inImpedances[idx[i]], s, c );
    }
    else
    {
      c = clGray;
      s = "n/a";
    }

    TRect r = rects[i];
    r.left += frame;
    r.top += frame;
    r.right -= frame;
    r.bottom -= frame;
    pBitmap->Canvas->Brush->Color = c;
    pBitmap->Canvas->FillRect( r );
    pBitmap->Canvas->TextRect(
      r,
      r.left + size.cx,
      ( r.top + r.bottom - size.cy ) / 2,
      ( labels[i] + s ).c_str()
    );
  }

  faSetBitmap( inID, pBitmap->Handle );
  delete pBitmap;
}

void
vAmpThread::ValueToText( float inValue, string& outText, TColor& outColor )
{
  const int bufSize = 100;
  char buf[bufSize];
  if ( inValue < 1000 )
  {
    snprintf( buf, bufSize, "%1.0f Ohm", inValue );
    outColor = clGreen;
  }
  else if ( inValue < 5000 )
  {
    snprintf( buf, bufSize, "%1.2f kOhm", inValue / 1e3 );
    outColor = clGreen;
  }
  else if ( inValue < 30e5 )
  {
    snprintf( buf, bufSize, "%1.1f kOhm", inValue / 1e3 );
    outColor = TColor( 0x0000a5FF );
  }
  else if ( inValue < 1e6 )
  {
    snprintf( buf, bufSize, "%1.1f kOhm", inValue / 1e3 );
    outColor = clRed;
  }
  else
  {
    snprintf( buf, bufSize, ">1 MOhm" );
    outColor = clPurple;
  }
  outText = buf;
}

Graphics::TBitmap*
vAmpThread::NewBitmap() const
{ // Create a bitmap in a format suited for the vAmp display.
  Graphics::TBitmap* pBitmap = new Graphics::TBitmap;
  pBitmap->PixelFormat = pf24bit;
  pBitmap->Width = 320;
  pBitmap->Height = 240;
  return pBitmap;
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
    short readTimes[25], blockSizes[25];
    int readPos = 0;

    memset(mBuffer, 0, mBufSize);

    int curChOffset = 0;
    int curCh=0, curSample = 0, chCount;
    map<int,int>::iterator mapIt;
    int waitTime;
    mPrevTime = PrecisionTime::Now();

    while (!this->IsTerminating() && mOk)
    {
        unsigned short tnow = PrecisionTime::Now();
        if (mMode == 3){  // GET IMPEDANCE AND CONTINUE
            for (size_t dev = 0; dev < mNumDevices; dev++){
                curChOffset = 0;
                mImpArray[dev].clear();
                int nChannels = m_nEEGChannels[dev]+1;
                unsigned int pBuf[17];
                for (int i = 0; i < nChannels; i++) pBuf[i] = 0;
                int nErrorCode = faGetImpedance(mDevList[dev], pBuf, sizeof(pBuf));

                for (int i = 0; i < nChannels; i++){
                    mImpArray[dev].push_back(float(pBuf[i]));
                }
                DisplayImpedances( mDevList[dev], mImpArray[dev] );
            }
            waitTime = min(mBlockSize/mSampleRate*1000 -
                PrecisionTime::TimeDiff(tnow, PrecisionTime::Now()),1000*mBlockSize/mSampleRate);
            if (waitTime > 0) Sleep(waitTime);
            SetEvent( acquireEventRead );
            continue;
        }
        //ACQUIRE DATA
        for (size_t dev = 0; dev < mNumDevices; dev++){
            curChOffset = 0;
            switch (m_nChannelMode[dev]){
                case DEVICE_CHANMODE_16:
                    pMaxBuffer = (char *)&m_tblMaxBuf16[dev][0];
                    nReadLen = m_nMaxPoints * sizeof(t_faDataModel16); // in bytes.
                    returnLen = ReadData(mDevList[dev], pMaxBuffer, nReadLen);
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
                        for (int sample = 0; sample < mBlockSize*mDecimate; sample++){
                            for (int ch = 0; ch < 16; ch++)
                            {
                                mapIt = mDevChMap[dev].find(ch);
                                if (mapIt != mDevChMap[dev].end()){
                                    mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) =
                                        (m_tblMaxBuf16[dev][sample].Main[ch] -
                                            ((mMode == 2) ? 0 : m_tblMaxBuf16[dev][sample].Main[16]))*m_tblChanInfo[dev][ch].dResolution;
                                }
                            }
                            mapIt = mDevChMap[dev].find(curChOffset+16);
                            if (mapIt != mDevChMap[dev].end())
                                mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = m_tblMaxBuf16[dev][sample].Aux[0]*m_tblChanInfo[dev][16].dResolution;
                            mapIt = mDevChMap[dev].find(curChOffset+17);
                            if (mapIt != mDevChMap[dev].end())
                                mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = m_tblMaxBuf16[dev][sample].Aux[1]*m_tblChanInfo[dev][16].dResolution;
                            //USHORT nTrigger = (tblMaxBuf16[dev][sample].Status >> 8) & 0x1;
                            //nTrigger |= (tblMaxBuf16[dev][sample].Status & 0xFF) << 1;
                            mapIt = mDevChMap[dev].find(curChOffset+18);
                            if (mapIt != mDevChMap[dev].end())
                                mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = m_tblMaxBuf16[dev][sample].Status & 0x1ff;
                        }
                        curChOffset += 19;
                    }
                    break;
                case DEVICE_CHANMODE_4:
                    pMaxBuffer = (char *)&m_tblMaxBuf4[dev][0];
                    nReadLen = m_nMaxPoints * sizeof(t_faDataFormatMode20kHz); // in bytes.
                    returnLen = ReadData(mDevList[dev], pMaxBuffer, nReadLen);
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
                        for (int sample = 0; sample < mBlockSize*mDecimate; sample++){
                            for (int ch = 0; ch < 5; ch++)
                            {
                                mapIt = mDevChMap[dev].find(ch);
                                if (mapIt != mDevChMap[dev].end()){
                                    mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) =
                                        (m_tblMaxBuf4[dev][sample].Main[ch])*m_tblChanInfo[dev][ch].dResolution;
                                }
                            }
                            //USHORT nTrigger = (tblMaxBuf4[dev][sample].Status >> 8) & 0x1;
                            //nTrigger |= (tblMaxBuf4[dev][sample].Status & 0xFF) << 1;
                            mapIt = mDevChMap[dev].find(curChOffset+4);
                            if (mapIt != mDevChMap[dev].end())
                                mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = m_tblMaxBuf4[dev][sample].Status & 0x1ff;
                        }
                    }
                    curChOffset += 5;
                    break;
                default:
                    pMaxBuffer = (char *)&m_tblMaxBuf8[dev][0];
                    nReadLen = m_nMaxPoints * sizeof(t_faDataModel8); // in bytes.
                    returnLen = ReadData(mDevList[dev], pMaxBuffer, nReadLen);
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
                        for (int sample = 0; sample < mBlockSize*mDecimate; sample++){
                            for (int ch = 0; ch < 8; ch++)
                            {
                                mapIt = mDevChMap[dev].find(ch);
                                if (mapIt != mDevChMap[dev].end()){
                                    mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) =
                                        (m_tblMaxBuf8[dev][sample].Main[ch] -
                                            ((mMode == 2) ? 0 : m_tblMaxBuf8[dev][sample].Main[8]))*m_tblChanInfo[dev][ch].dResolution;
                                }
                            }
                            mapIt = mDevChMap[dev].find(curChOffset+8);
                            if (mapIt != mDevChMap[dev].end())
                                mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = m_tblMaxBuf8[dev][sample].Aux[0]*m_tblChanInfo[dev][16].dResolution;
                            mapIt = mDevChMap[dev].find(curChOffset+9);
                            if (mapIt != mDevChMap[dev].end())
                                mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = m_tblMaxBuf8[dev][sample].Aux[1]*m_tblChanInfo[dev][16].dResolution;
                            //USHORT nTrigger = (tblMaxBuf16[dev][sample].Status >> 8) & 0x1;
                            //nTrigger |= (tblMaxBuf16[dev][sample].Status & 0xFF) << 1;
                            mapIt = mDevChMap[dev].find(curChOffset+10);
                            if (mapIt != mDevChMap[dev].end())
                                mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = m_tblMaxBuf8[dev][sample].Status & 0x1ff;
                        }
                    }
                    curChOffset += 11;
                    break;
            }
            t_faDataState state = { sizeof( t_faDataState ), };
            faGetDataState( mDevList[dev], &state );
            if( state.DataCounterErrors != mDataCounterErrors[dev] )
            {
                Lock();
                mWarnings << "Amplifier #" << dev << " reports data loss. "
                          << "The total number of counter errors is "
                          << state.DataCounterErrors
                          << endl;
                Unlock();
                mDataCounterErrors[dev] = state.DataCounterErrors;
            }
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
#endif
    int tdiff, remTime;
    DWORD sleepTime;
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

