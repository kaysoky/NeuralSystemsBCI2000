////////////////////////////////////////////////////////////////////////////////
// $Id: vAmpThread.h 2032 2008-06-26 17:11:32Z mellinger $
// Author: jadamwilson2@gmail.com
// Description: A class that encapsulates a data acquisition thread for
//   vAmp devices
//
// (C) 2000-2009, BCI2000 Project
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
#include "impedanceGUI.h"

using namespace std;
class vAmpThread : public OSThread
{
 public:
  vAmpThread( int inBlockSize, float sampleRate, int decimate, vector<int> chList, int chsPerDev[MAX_ALLOWED_DEVICES], vector<int> devList, int mode, float hpCorner);
  virtual ~vAmpThread();

  float ExtractData(int ch, int sample);
  string lastErr(){return mLastErr.str();}
  bool ok(){return mOk;}
  void AdvanceReadBlock();
  HANDLE acquireEventRead;

 private:
	virtual int Execute();
	bool init();

	int    mBlockSize,
		 mTimeout,
		 mBufSize,
		 mNumPoints,
		 mWriteCursor,
		 mReadCursor,
		 mRingBufferSize,
		 mDecimate,
		 mNumChannels,
		 mAnalogChannels;
	float  mSampleRate;
	unsigned int mImpBuf[17];
	unsigned short mPrevTime;
	float mHPcorner;
	stringstream mLastErr;
	vector<int> mChList;
	int mMode;
	map<int, int> mDevChRevMap[MAX_ALLOWED_DEVICES], mDevChMap[MAX_ALLOWED_DEVICES];
	int mDevChs[MAX_ALLOWED_DEVICES];
	//int mDigChs[MAX_ALLOWED_DEVICES];
	GenericSignal mDataBuffer, mDataOutput;

	vector< vector<float> > mImpArray;
	valarray< valarray<float> >	mTrigBuffer; //mdatabuffer[device][ch][sample]
	float *mBuffer;

	HANDLE mEvent,
		 mDev;
	bool mOk;
	TimpGUI *mImpGui;
	int mChsPerDev[MAX_ALLOWED_DEVICES];
	unsigned int mNumDevices;
	int mDevIds[MAX_ALLOWED_DEVICES];
	vector<int> mDevList;
	int m_nChannelMode[MAX_ALLOWED_DEVICES];
	int m_nEEGChannels[MAX_ALLOWED_DEVICES];
	int m_nAUXChannels[MAX_ALLOWED_DEVICES];
	bool m_bOpen[MAX_ALLOWED_DEVICES];
	int mStartMode;
	int mBufferSize;
	t_faInformation m_DeviceInfo[MAX_ALLOWED_DEVICES];		// Device info.
	t_faProperty	m_DeviceProp[MAX_ALLOWED_DEVICES];		// Channel properties.
	vector<CChannelInfo>
					m_tblChanInfo[MAX_ALLOWED_DEVICES];


	vector<t_faDataModel8>	
					tblMaxBuf8[MAX_ALLOWED_DEVICES];		// 1 read cycle buffer of 8 channel system + 2 add. samples.
	vector<t_faDataModel16> 
					tblMaxBuf16[MAX_ALLOWED_DEVICES];	// 1 read cycle buffer of 16 channel system + 2 add. samples.
	vector<t_faDataFormatMode20kHz> 
					tblMaxBuf4[MAX_ALLOWED_DEVICES];		// 1 read cycle buffer (highspeed, 4 ch) + 2 add. samples.
	vector<float>	tblEEGData[MAX_ALLOWED_DEVICES];		// 1 read cycle of only EEG and AUX signals.
	vector<float>	tblTrigger[MAX_ALLOWED_DEVICES];		// 1 read cycle of only Trigger signals.
	vector<float>	tblPacket[MAX_ALLOWED_DEVICES];		// 1 read cycle of EEG, AUX, Trigger signals.
	t_faDataModeSettings mFastSettings[MAX_ALLOWED_DEVICES];
	t_faDataMode mDataMode;
	bool mHighSpeed;
	set<int> mDigChs;

	int ReadData(int nDeviceId, char *pBuffer, int nReadLen);
	IIRFilter<float> mFilter;
	typedef Ratpoly<FilterDesign::Complex> TransferFunction;

	FILE *logFile;
};

#endif // VAMP_THREAD_H
//---------------------------------------------------------------------------
