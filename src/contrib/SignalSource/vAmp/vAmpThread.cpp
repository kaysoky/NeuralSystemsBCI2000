////////////////////////////////////////////////////////////////////////////////
// $Id: vAmpThread.h 2032 2008-06-26 17:11:32Z mellinger $
// Author: jadamwilson2@gmail.com
// Description: A class that encapsulates a data acquisition thread for
//   vAmp devices
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIError.h"
#include "PrecisionTime.h"
#include "vAmpThread.h"
#include "FirstAmp.h"

using namespace std;

vAmpThread::vAmpThread( int inBlockSize, float sampleRate, int decimate, vector<int> chList, int chsPerDev[MAX_ALLOWED_DEVICES], vector<int> devList, int mode, float hpCorner)
: OSThread( true ),
  mBlockSize( inBlockSize ),
  mSampleRate(sampleRate),
  mWriteCursor( 0 ),
  mReadCursor( 0 ),
  mBuffer(NULL),
  mImpGui(NULL),
  mDecimate(decimate),
  acquireEventRead(NULL),
  mNumDevices(0),
  mDevList(devList),
  mChList(chList),
  mMode(mode),
  mHPcorner(hpCorner),
  mEvent( NULL )
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
  if (mNumDevices > MAX_ALLOWED_DEVICES) {
		mLastErr << "A maximum of 4 devices can be present on the system at a time."<<endl;
		mOk =false;
	  return;
  }

  //open the devices
  mNumChannels = 0;
  mRingBufferSize = int(mBlockSize*1);
  mTrigBuffer.resize(MAX_ALLOWED_DEVICES);

  int nMaxPoints = mBlockSize*decimate;
  if (mMode == 3){
	nMaxPoints = 1;
	mImpGui = new TimpGUI(NULL);
	mImpGui->setSize(mNumDevices, 17);
	mImpGui->Show();
	mImpArray.resize(mNumDevices);
  }

  for (int dev = 0; dev < mDevList.size(); dev++)
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
			tblMaxBuf8[dev].resize(nMaxPoints);
			m_tblChanInfo[dev].resize(m_nEEGChannels[dev] + m_nAUXChannels[dev] + 1); // 1 Trigger.
			faSetDataMode(mDevList[dev], dmNormal, NULL);
			break;
		case FA_MODEL_16:
			m_nChannelMode[dev] = DEVICE_CHANMODE_16;
			tblMaxBuf16[dev].resize(nMaxPoints);
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
		tblMaxBuf4[dev].resize(nMaxPoints);
		m_nEEGChannels[dev] = FA_MODE_20_KHZ_CHANNELS_MAIN; // EEG
		m_nAUXChannels[dev] = 0;	// no AUX.
		m_tblChanInfo[dev].resize(m_nEEGChannels[dev] + m_nAUXChannels[dev] + 1); // 1 Trigger.
		faSetDataMode(mDevList[dev], dm20kHz4Channels, mFastSettings);
	}
	for (int ch = 0; ch < mChList.size(); ch++)
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
	tblEEGData[dev].resize((m_nEEGChannels[dev] + m_nAUXChannels[dev])*mBlockSize*decimate);
	tblTrigger[dev].resize(mBlockSize);
	tblPacket[dev].resize((m_nEEGChannels[dev] + m_nAUXChannels[dev]+1)*mBlockSize*decimate); // actual results.
	mNumChannels += mChsPerDev[dev];
  }
  mDataBuffer.SetProperties(SignalProperties(mChList.size(), mBlockSize*decimate, SignalType::float32));
  mDataOutput = mDataBuffer;
  mBufSize = mChList.size()*mRingBufferSize;
  mBuffer = new float[mBufSize];

  TransferFunction tf(1.0);
  float outGain = 1.0;
  TransferFunction lp =
		  FilterDesign::Butterworth().Order( 2 ).Lowpass(.45).TransferFunction();
  if (mHPcorner > 0){
	TransferFunction hp  =
		  FilterDesign::Butterworth().Order( 2 ).Highpass(mHPcorner).TransferFunction();
	tf *= hp;
	outGain /= abs(hp.Evaluate(-1.0));
  }
  tf *= lp;

  outGain /= abs( lp.Evaluate( 1.0 ) ); 

  mFilter.SetZeros(tf.Numerator().Roots())
		.SetPoles(tf.Denominator().Roots())
		.SetGain(outGain)
		.Initialize();

  acquireEventRead = CreateEvent(NULL, true, false, "ReadEvent");

  //logFile = fopen("c:\\vampthread.txt","w");
}

vAmpThread::~vAmpThread()
{
  delete[] mBuffer;
  delete mImpGui;
  CloseHandle(acquireEventRead);
  for (int dev = 0; dev < mNumDevices; dev++){
		faStop(mDevList[dev]);
		faClose(mDevList[dev]);
	}
  //fclose(logFile);
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
	for (int dev = 0; dev < mNumDevices; dev++){
		mFilter.Initialize(mDataBuffer.Channels());
		faStart(mDevList[dev]);
		switch (mMode){
			case 2:
			case 4:
				faStartCalibration(mDevList[dev]);
				break;
			case 3:				
				faStartImpedance(mDevList[dev]);
				break;
		}
	}
	

	bool doImpedance = (mMode == 3);
	int curChOffset = 0;
	int curCh=0, curSample = 0, chCount;
	map<int,int>::iterator mapIt;
	int waitTime;
	mPrevTime = PrecisionTime::Now();

	while (!this->IsTerminating() && mOk)
	{
		unsigned short tnow = PrecisionTime::Now();
		if (mMode == 3){  // GET IMPEDANCE AND CONTINUE

			for (int dev = 0; dev < mNumDevices; dev++){
				curChOffset = 0;
				mImpArray[dev].clear();
				switch (m_nChannelMode[dev]){
					case DEVICE_CHANMODE_16:
						for (int i = 0; i < 17; i++) mImpBuf[i] = 0;
						int nErrorCode = faGetImpedance(mDevList[dev], mImpBuf,(m_nEEGChannels[dev]+1) * sizeof(UINT));

						for (int i = 0; i < 17; i++){
							mImpArray[dev].push_back(float(mImpBuf[i]));
						}
						break;
				}
			}
			mImpGui->setGrid(mImpArray);
			waitTime = max(mBlockSize/mSampleRate*1000 -
				PrecisionTime::TimeDiff(tnow, PrecisionTime::Now()),1000*mBlockSize/mSampleRate);
			if (waitTime > 0) Sleep(waitTime);
            SetEvent( acquireEventRead );
			continue;
		}
		//ACQUIRE DATA
		for (int dev = 0; dev < mNumDevices; dev++){
			curChOffset = 0;
			switch (m_nChannelMode[dev]){
				case DEVICE_CHANMODE_16:
					pMaxBuffer = (char *)&tblMaxBuf16[dev][0];
					nReadLen = int(tblMaxBuf16[dev].size() * sizeof(t_faDataModel16)); // in bytes.
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
										(tblMaxBuf16[dev][sample].Main[ch] -
											((mMode == 2) ? 0 : tblMaxBuf16[dev][sample].Main[16]))*m_tblChanInfo[dev][ch].dResolution;
								}
							}
							mapIt = mDevChMap[dev].find(curChOffset+16);
							if (mapIt != mDevChMap[dev].end())
								mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = tblMaxBuf16[dev][sample].Aux[0]*m_tblChanInfo[dev][16].dResolution;
							mapIt = mDevChMap[dev].find(curChOffset+17);
							if (mapIt != mDevChMap[dev].end())
								mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = tblMaxBuf16[dev][sample].Aux[1]*m_tblChanInfo[dev][16].dResolution;
							//USHORT nTrigger = (tblMaxBuf16[dev][sample].Status >> 8) & 0x1;
							//nTrigger |= (tblMaxBuf16[dev][sample].Status & 0xFF) << 1;
							mapIt = mDevChMap[dev].find(curChOffset+18);
							if (mapIt != mDevChMap[dev].end())
								mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = tblMaxBuf16[dev][sample].Status & 0x1ff;
						}
						curChOffset += 19;
					}
					
					break;
				case DEVICE_CHANMODE_4:
					pMaxBuffer = (char *)&tblMaxBuf4[dev][0];
					nReadLen = int(tblMaxBuf4[dev].size() * sizeof(t_faDataFormatMode20kHz)); // in bytes.
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
										(tblMaxBuf4[dev][sample].Main[ch])*m_tblChanInfo[dev][ch].dResolution;
								}
							}
							
							//USHORT nTrigger = (tblMaxBuf4[dev][sample].Status >> 8) & 0x1;
							//nTrigger |= (tblMaxBuf4[dev][sample].Status & 0xFF) << 1;
							mapIt = mDevChMap[dev].find(curChOffset+4);
							if (mapIt != mDevChMap[dev].end())
								mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = tblMaxBuf4[dev][sample].Status & 0x1ff;
						}
					}
					curChOffset += 5;
					break;
				default:
					pMaxBuffer = (char *)&tblMaxBuf8[dev][0];
					nReadLen = int(tblMaxBuf8[dev].size() * sizeof(t_faDataModel8)); // in bytes.
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
										(tblMaxBuf8[dev][sample].Main[ch] -
											((mMode == 2) ? 0 : tblMaxBuf8[dev][sample].Main[8]))*m_tblChanInfo[dev][ch].dResolution;
								}
							}
							mapIt = mDevChMap[dev].find(curChOffset+8);
							if (mapIt != mDevChMap[dev].end())
								mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = tblMaxBuf8[dev][sample].Aux[0]*m_tblChanInfo[dev][16].dResolution;
							mapIt = mDevChMap[dev].find(curChOffset+9);
							if (mapIt != mDevChMap[dev].end())
								mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = tblMaxBuf8[dev][sample].Aux[1]*m_tblChanInfo[dev][16].dResolution;
							//USHORT nTrigger = (tblMaxBuf16[dev][sample].Status >> 8) & 0x1;
							//nTrigger |= (tblMaxBuf16[dev][sample].Status & 0xFF) << 1;
							mapIt = mDevChMap[dev].find(curChOffset+10);
							if (mapIt != mDevChMap[dev].end())
								mDataBuffer(mDevChRevMap[dev][mapIt->second],sample) = tblMaxBuf8[dev][sample].Status & 0x1ff;
						}
					}
					curChOffset += 11;
					break;
			}
			//FILTER
			mFilter.Process(mDataBuffer, mDataOutput);
				
			//DECIMATE
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
			if (mMode == 3){
				mMode= 0;
				doImpedance = false;
				for (int i = 0; i < mDevList.size(); i++)
					faStopImpedance(mDevList[i]);
			}

		}
		SetEvent( acquireEventRead );
	}

 	for (int dev = 0; dev < mNumDevices; dev++){
		switch (mMode){
			case 2:
				faStopCalibration(mDevList[dev]);

				break;
			case 3:
				faStopImpedance(mDevList[dev]);
				break;
		}
		faStop(mDevList[dev]);
		faClose(mDevList[dev]);
	}

	return 0;
}

int vAmpThread::ReadData(int nDeviceId, char *pBuffer, int nReadLen)
{
	int nReturnLen = 0,			// Current return length in bytes.
		nLenToRead = nReadLen;	// len in bytes.
	int nLoops = 0;
	unsigned short readTime, startTime = PrecisionTime::Now();
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
		readTime = PrecisionTime::TimeDiff(startTime, PrecisionTime::Now());
		if (readTime >(int)((float(2*mBlockSize)*1000)/(mSampleRate)) && nLenToRead > 0)
			return -1;
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
			tdiff = PrecisionTime::TimeDiff(mPrevTime, PrecisionTime::Now());
			remTime = (int)((float(mBlockSize)*1000)/(mSampleRate)) - (tdiff);
			if (remTime >0 && remTime < (float(mBlockSize)*1000)/(mSampleRate)) {
				sleepTime = remTime;
				Sleep(0);
				//fprintf(logFile,"(S%d) ", remTime);
			}
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
