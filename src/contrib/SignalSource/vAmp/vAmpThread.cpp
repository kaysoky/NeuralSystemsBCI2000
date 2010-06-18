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
	
	m_tblMaxBuf4[ 0 ] = NULL;
	m_tblMaxBuf8[ 0 ] = NULL;
	m_tblMaxBuf16[ 0 ] = NULL;
	mDataCounterErrors = 0;
	

	mOk = true;
	mNumDevices = faGetCount();
	mLastErr.str("");
	mHighSpeed = mMode == 1 || mMode == 4;
	if (mNumDevices < 1) 
	{
		mLastErr <<"No vAmp devices were found."<<endl;
		mOk =false;
		return;
	}
	if (mNumDevices > MAX_ALLOWED_DEVICES)
	{
		mLastErr << "A maximum of 1 devices can be present on the system at a time."<<endl;
		mOk =false;
		return;
	}

	//open the devices
	mNumChannels = 0;
	mRingBufferSize = int(mBlockSize*cBlocksInRingBuffer);
	mTrigBuffer.resize(MAX_ALLOWED_DEVICES);

	m_nMaxPoints = mBlockSize*decimate;
	if (mMode == 3)
	{
		m_nMaxPoints = 1;
		mImpArray.resize(mNumDevices);
	}
	
	if (faOpen(mDevList[0]) != FA_ERR_OK)
	{
		mLastErr << "Error opening device " << mDevList[0] << endl;
		mOk =false;
		return;
	}
   
    memset(&m_DeviceInfo[0], 0, sizeof(m_DeviceInfo[0]));  
   
    mDigChs.clear();
    if (faGetInformation(mDevList[0], &(m_DeviceInfo[0])) != FA_ERR_OK)
    {
        mLastErr << "Failed getting device information for " << mDevList[0]<<endl;
        mOk =false;
        return;
    }
    switch (m_DeviceInfo[0].Model)
    {
        case FA_MODEL_8:
            m_nChannelMode = DEVICE_CHANMODE_8;
            m_nEEGChannels = FA_MODEL_8_CHANNELS_MAIN - 1; // without trigger
            m_nAUXChannels= FA_MODEL_8_CHANNELS_AUX;
            delete[] m_tblMaxBuf8[0];
            m_tblMaxBuf8[0] = new t_faDataModel8[m_nMaxPoints];
            m_tblChanInfo[0].resize(m_nEEGChannels + m_nAUXChannels+ 1); // 1 Trigger.            
            if(faSetDataMode(mDevList[0], dmNormal, NULL)!=FA_ERR_OK)
            {
                 mLastErr << "Error setting data mode."<<endl;
                 mOk =false;
                 return;	
            }
            mIs8Channel  = true;
            break;
        case FA_MODEL_16:
            m_nChannelMode = DEVICE_CHANMODE_16;
            delete[] m_tblMaxBuf16[0];
            m_tblMaxBuf16[0] = new t_faDataModel16[m_nMaxPoints];
            m_nEEGChannels = FA_MODEL_16_CHANNELS_MAIN - 1; // without trigger
            m_nAUXChannels = FA_MODEL_16_CHANNELS_AUX;
            m_tblChanInfo[0].resize(m_nEEGChannels + m_nAUXChannels + 1); // 1 Trigger.            
            if(faSetDataMode(mDevList[0], dmNormal, NULL)!=FA_ERR_OK)
            {
                 mLastErr << "Error setting data mode."<<endl;
                 mOk =false;
                 return;	
            }
            mIs8Channel = false;
            break;
        default: // Unknow device (error).
            mLastErr << "Unknown device model for device " << mDevList[0] << endl;
            mOk =false;
          return;
    }

    if (mHighSpeed)
    {
        m_nChannelMode = DEVICE_CHANMODE_4;
        delete[] m_tblMaxBuf4[0];
        m_tblMaxBuf4[0] = new t_faDataFormatMode20kHz[m_nMaxPoints];
        m_nEEGChannels = FA_MODE_20_KHZ_CHANNELS_MAIN; // EEG
        m_nAUXChannels = 0;	// no AUX.
        m_tblChanInfo[0].resize(m_nEEGChannels + m_nAUXChannels + 1); // 1 Trigger.
        if(faSetDataMode(mDevList[0], dm20kHz4Channels, mFastSettings)!=FA_ERR_OK)
        {
              mLastErr << "Error setting data mode."<<endl;
              mOk =false;
              return;	
        }    
    }
    for (size_t ch = 0; ch < mChList.size(); ch++)
    {    
		if (mMode == 1)
		{
			mFastSettings[0].Mode20kHz4Channels.ChannelsPos[mChList[ch] ] = mChList[ch];
			mFastSettings[0].Mode20kHz4Channels.ChannelsNeg[mChList[ch] ] = -1;
		}
		if (mChList[ch] ==  (m_nEEGChannels + m_nAUXChannels))
		{
			mDigChs.insert(ch);
		}              
	}
	
	// Retrieves device properties.
    memset(&m_DeviceProp[0], 0, sizeof(m_DeviceProp[0]));
    if (faGetProperty(mDevList[0], &m_DeviceProp[0]) != FA_ERR_OK)
    {
        mLastErr << "Error getting device properties for device " << mDevList[0]<<endl;
        mOk =false;
        return;
    }

    // Channel type, channel resolution.
    for (UINT i = 0; i < m_tblChanInfo[0].size(); i++)
    {
        CChannelInfo& ci = m_tblChanInfo[0][i];
        if (i < UINT(m_nEEGChannels)) // EEG
        {
            ci.nType = DEVICE_CHAN_TYPE_EEG;
            ci.dResolution = double(m_DeviceProp[0].ResolutionEeg * 1e6); // V > µV
        }
        else if (m_nAUXChannels > 0 && // AUX present
            i >= UINT(m_nEEGChannels) && i < m_tblChanInfo[0].size() -1 ) // AUX
        {
            ci.nType = DEVICE_CHAN_TYPE_AUX;
            ci.dResolution = double(m_DeviceProp[0].ResolutionAux * 1e6); // V > µV
        }
        else // Digital port.
        {
            ci.nType = DEVICE_CHAN_TYPE_TRIGGER;
            ci.dResolution = 1.0f;
        }        
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

	if (hpCorner > 0)
	{
		TransferFunction hp  = FilterDesign::Butterworth().Order( 2 ).Highpass(hpCorner/mDecimate).TransferFunction();
		outGain /= abs(hp.Evaluate(-1.0));
		tf *= hp;
	}

	mFilter.SetZeros(tf.Numerator().Roots())
			.SetPoles(tf.Denominator().Roots())
			.SetGain(outGain)
			.Initialize(mDataBuffer.Channels());

	acquireEventRead = CreateEvent(NULL, true, false, "ReadEvent");
	
	if(mMode!=3) DisplayBCI2000Logo( mDevList[0] );
	t_faDataState state = { sizeof( t_faDataState ), };
	faGetDataState( mDevList[0], &state );
	mDataCounterErrors = state.DataCounterErrors;
	// faStart/faStop etc must be called from the main thread.
	if (faStart(mDevList[0])!= FA_ERR_OK)
	{
	     {			   	
                    mLastErr << "Error starting start vAmp device."<<endl;
                    mOk =false;
                    return;
  	     }	
	}
	switch (mMode)
	{
		case 0:
		case 1:
			break;
		case 2:
		case 4:
			if(faStartCalibration(mDevList[0])!=FA_ERR_OK)
			{
			     mLastErr << "Error starting calibration."<<endl;
                             mOk =false;
                             return;
                        }
			break;
		case 3:			
			if(faStartImpedance(mDevList[0])!= FA_ERR_OK)
			{			   	
                             mLastErr << "Error starting impedance measurement."<<endl;
                             mOk =false;
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
	ClearAmpDisplay( mDevList[0] );
	switch (mMode)
	{
		case 0:
		case 1:
			break;
		case 2:
		case 4:
			faStopCalibration(mDevList[0]);
			break;
		case 3:
			faStopImpedance(mDevList[0]);
			break;
	}
	faStop(mDevList[0]);
	faClose(mDevList[0]);
	

	
	delete[] m_tblMaxBuf4[0];
	delete[] m_tblMaxBuf8[0];
	delete[] m_tblMaxBuf16[0];
	
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

unsigned int  vAmpThread::DisplayImpedances( int inID, const vector<float>& inImpedances )
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
  //vAmp API function 
  unsigned int status = faSetBitmap( inID, pBitmap->Handle );
  delete pBitmap;
  return status;
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

    
    int  curSample = 0, chCount;    
    int waitTime;
    mPrevTime = PrecisionTime::Now();
    unsigned short tLastDisplayUpdate = PrecisionTime::Now();
	bool bIsStart = true;
	
    while (!this->IsTerminating() && mOk)
    {
        unsigned short tnow = PrecisionTime::Now();
        if (mMode == 3)
	{   
	    // GET IMPEDANCE AND CONTINUE           
            mImpArray[0].clear();
            if(mIs8Channel)
            {
                unsigned int pBuf[FA_MODEL_8_CHANNELS_MAIN];
                for (int i = 0; i < FA_MODEL_8_CHANNELS_MAIN; i++) {pBuf[i] = 0;}
                if(faGetImpedance(mDevList[0], pBuf, sizeof(pBuf))==FA_ERR_OK)
                {
                   for (int i = 0; i < FA_MODEL_8_CHANNELS_MAIN; i++)
                   {
                        mImpArray[0].push_back(float(pBuf[i]));
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
                if(faGetImpedance(mDevList[0], pBuf, sizeof(pBuf))==FA_ERR_OK)
                {
                   for (int i = 0; i < FA_MODEL_16_CHANNELS_MAIN; i++)
                   {
                        mImpArray[0].push_back(float(pBuf[i]));
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
		 if(DisplayImpedances( mDevList[0], mImpArray[0] )!=FA_ERR_OK)
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
                pMaxBuffer = (char *)&m_tblMaxBuf16[0][0];
                nReadLen = m_nMaxPoints * sizeof(t_faDataModel16); // in bytes.
                returnLen = ReadData(mDevList[0], pMaxBuffer, nReadLen);
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
                    for (int sample = 0;  sample< mBlockSize*mDecimate; sample++)
                    {
                        for (int chPos = 0; chPos < (int)mChList.size(); chPos++)
                        {
                            int nPos = mChList[chPos];//the channel of the device that should appear in this position according to parameter definition
                            // Process input of normal data channels (subtract reference if not in calibration mode                            
                            if(0<=nPos&& nPos<16)
                            {
                                mDataBuffer(chPos,sample) =
                                    (m_tblMaxBuf16[0][sample].Main[nPos] - ((mMode == 2) ? 0 : m_tblMaxBuf16[0][sample].Main[16]))*m_tblChanInfo[0][nPos].dResolution;
                                	
                            }
                            //auxiliary channel 1
                            if(nPos ==16) mDataBuffer(chPos,sample) = m_tblMaxBuf16[0][sample].Aux[0]*m_tblChanInfo[0][16].dResolution;
                            //auxiliary channel 2
                            if(nPos ==17) mDataBuffer(chPos,sample) = m_tblMaxBuf16[0][sample].Aux[1]*m_tblChanInfo[0][16].dResolution;
                            //Trigger Channel
                            if(nPos==18)  mDataBuffer(chPos,sample) = m_tblMaxBuf16[0][sample].Status & 0x1ff;                                
                        }
                    }                                               
                }
                break;
            case DEVICE_CHANMODE_4:
                pMaxBuffer = (char *)&m_tblMaxBuf4[0][0];
                nReadLen = m_nMaxPoints * sizeof(t_faDataFormatMode20kHz); // in bytes.
                returnLen = ReadData(mDevList[0], pMaxBuffer, nReadLen);
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
                                mDataBuffer(chPos,sample) = (m_tblMaxBuf4[0][sample].Main[nPos])*m_tblChanInfo[0][nPos].dResolution;
                            }
                            //trigger channel
                            if(nPos ==4)mDataBuffer(chPos,sample) = m_tblMaxBuf4[0][sample].Status & 0x1ff;                                
                        }                           
                    }
                }                   
                break;
            default:
                pMaxBuffer = (char *)&m_tblMaxBuf8[0][0];
                nReadLen = m_nMaxPoints * sizeof(t_faDataModel8); // in bytes.
                returnLen = ReadData(mDevList[0], pMaxBuffer, nReadLen);
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
                                    (m_tblMaxBuf8[0][sample].Main[nPos] - ((mMode == 2) ? 0 : m_tblMaxBuf8[0][sample].Main[8]))*m_tblChanInfo[0][nPos].dResolution;                                
                            }
                            // Auxiliary channel 1
                            if(nPos ==8)mDataBuffer(chPos,sample) = m_tblMaxBuf8[0][sample].Aux[0]*m_tblChanInfo[0][8].dResolution;
                            // Auxiliary channel 2
                            if(nPos ==9)mDataBuffer(chPos,sample) = m_tblMaxBuf8[0][sample].Aux[1]*m_tblChanInfo[0][8].dResolution;
                            //Trigger channel
                            if(nPos ==10)mDataBuffer(chPos,sample) = m_tblMaxBuf8[0][sample].Status & 0x1ff;
                        }
                    }
                        
                }               
                break;
        }
        t_faDataState state = { sizeof( t_faDataState ), };
        faGetDataState( mDevList[0], &state );
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

