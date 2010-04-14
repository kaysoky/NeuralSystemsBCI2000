////////////////////////////////////////////////////////////////////////////////
// $Id: vAmpADC.cpp 1967 2008-05-19 15:00:13Z awilson $
// Author: jadamwilson2@gmail.com
// Description: BCI2000 Source Module for BrainProducts V-Amp devices.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "vAmpADC.h"
#include <stdlib.h>

#include "defines.h"
#include "GenericSignal.h"
#include "MeasurementUnits.h"

#include <algorithm>

using namespace std;

// Register the source class with the framework.
RegisterFilter( vAmpADC, 1 );

// **************************************************************************
// Function:   vAmpADC
// Purpose:    The constructor for the vAmpADC
// **************************************************************************
vAmpADC::vAmpADC()
: mAcquire(NULL)
{

  // add all the parameters that this ADC requests to the parameter list
  BEGIN_PARAMETER_DEFINITIONS
   "Source int SourceCh=      19 16 1 128 "
	   "// number of digitized channels total",
   "Source int SampleBlockSize= 8 5 1 20000 "
	   "// number of samples per block",
   "Source int SamplingRate=    2000 2000 100 20000 "
	   "// the signal sampling rate",
   "Source intlist SourceChList= 0 0 1 128 "
	   "// list of channels to digitize",
   "Source intlist SourceChDevices=  1 19 19 1 19 "
       "// number of digitized channels per device",
   "Source float HPFilterCorner= 0.1 0 0 % "
   		"// high-pass filter corner (use 0 or leave blank to disable)",
   "Source int AcquisitionMode=      0 0 0 4 "
        "// data acquisition mode: "
			" 0: analog signal acquisition,"
			" 1: high-speed signal acquisition,"
			" 2: calibration,"
			" 3: impedance,"
			" 4: high-speed calibration"
			" (enumeration)",
   "Source list DeviceIDs= 0 % "
       "// list of V-Amps to be used",
  END_PARAMETER_DEFINITIONS
}


vAmpADC::~vAmpADC()
{
	Halt();
}


// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// **************************************************************************
void vAmpADC::Preflight( const SignalProperties&,
                                  SignalProperties& outSignalProperties ) const
{
	// Requested output signal properties.
	bool tHighSpeed = Parameter("AcquisitionMode") == 1 || Parameter("AcquisitionMode") == 4;
	SignalType signalType = SignalType::float32;
	outSignalProperties = SignalProperties(
	   Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), signalType );

	// Parameter consistency checks: Existence/Ranges and mutual Ranges.
	if ( Parameter("SourceChList")->NumValues() > 0 )
	{
		if (Parameter("SourceChList")->NumValues() != Parameter("SourceCh"))
		{
			bcierr << "# elements in SourceChList must match total # channels (SourceCh)" <<endl;
			return;
		}
	}
	if( Parameter("SourceChGain")->NumValues() != Parameter("SourceCh"))
	{
	bcierr << "# elements in SourceChGain has to match total # channels" << endl;
	return;
	}
	if( Parameter("SourceChOffset")->NumValues() != Parameter("SourceCh"))
	{
	bcierr << "# elements in SourceChOffset has to match total # channels" << endl;
	return;
	}

	bool goodSourceChGain = true,
	   goodSourceChOffset = true;
	for (int ch=0; ch<Parameter("SourceCh"); ch++)
	{
	goodSourceChGain = goodSourceChGain && ( Parameter("SourceChGain")(ch) > 0 );
	goodSourceChOffset = goodSourceChOffset && ( fabs(float(Parameter("SourceChOffset")(ch))) < 0.0001 );
	}
	if( !goodSourceChGain )
	bcierr << "SourceChGain is not supposed to be zero" << endl;
	if( !goodSourceChOffset )
	bcierr << "SourceChOffset is supposed to be zero" << endl;

	//get the number of devices
	UINT tNumDevices = FA_ID_INVALID;
	tNumDevices = faGetCount();
	if (tNumDevices < 1) {
	  bcierr <<"No vAmp devices were found."<<endl;
	}
	if (tNumDevices > MAX_ALLOWED_DEVICES) {
		bcierr << "A maximum of " << MAX_ALLOWED_DEVICES << " devices can be present on the system at a time."<<endl;
		return;     
	}

	//get device IDs
	int tDevIds[MAX_ALLOWED_DEVICES];
	t_faInformation tDeviceInfo[MAX_ALLOWED_DEVICES];
	for (int i = 0; i < tNumDevices; i++){
		tDevIds[i] = faGetId(i);
		if (tDevIds[i] == FA_ID_INVALID)
			bcierr << "Invalid device ID reported: "<< tDevIds[i] << endl;
		faGetInformation(tDevIds[i], &(tDeviceInfo[i]));
		bciout << "Device found with serial: "<< tDeviceInfo[i].SerialNumber << endl;
	}
	
	if (Parameter("DeviceIDs")->NumValues() != tNumDevices){
		bcierr << "Device serial numbers must be listed in DeviceIDs"<<endl;
		return;
	}


	if (Parameter("SamplingRate") < 200){
		bcierr << "The sample rate must be >= 200 Hz (2000Hz in high-speed)" << endl;
		return;
	}
	int dectmp = getDecimation();
	if (dectmp == 0)
	{
		if (!tHighSpeed){
		bcierr << "Invalid sample rate. Valid values are 2000, 1000, "
			<<float(2000.0/3)
			<<", 500, 400, "
			<< 2000.0/6 << ", "
			<< 2000.0/7 << ", "
			<< 2000.0/8 << ", "
			<< 2000.0/9 << ", or 200"<<endl;
		return;
		}
		else{
			bcierr << "Invalid sample rate. Valid values in high-speed mode are 20000, 10000, "
				<<float(20000.0/3)
				<<", 5000, 4000, "
				<< 20000.0/6 << ", "
				<< 20000.0/7 << ", "
				<< 20000.0/8 << ", "
				<< 20000.0/9 << ", or 2000"<<endl;
			return;
		}
	}

	if (Parameter("DeviceIDs")->NumValues() > MAX_ALLOWED_DEVICES){
		bcierr << "The maximum number of devices allowed is " << MAX_ALLOWED_DEVICES<<endl;
		return;
	}
	
	if( Parameter("DeviceIDs")->NumValues() != Parameter("SourceChDevices")->NumValues() )
	{
	bcierr << "# devices has to equal # entries in SourceChDevices" << endl;
	return;
	}
  
	int totalnumchannels=0;
	for (int dev=0; dev<Parameter("DeviceIDs")->NumValues(); dev++){
		if (tHighSpeed){
			if (Parameter("SourceChDevices")(dev) > 5){
				bcierr << "In high-speed acquisition mode, devices can have at most 5 channels (4 EEG + 1 Dig)." <<endl;
				return;
			}
		}
		totalnumchannels += Parameter("SourceChDevices")(dev);
		bool tFound = false || (tNumDevices == 1);
		for (int d = 0; d < tNumDevices; d++){
			if (Parameter("DeviceIDs")(dev) == tDeviceInfo[d].SerialNumber)
				tFound = true;
		}
		if (!tFound){
			bcierr << "Device with serial " << Parameter("DeviceIDs")(dev) << " not found"<<endl;
			return;
		}
	}
	if( Parameter("SourceCh") != totalnumchannels )	{
		bcierr << "# total channels ("<< totalnumchannels<<") has to equal sum of all channels over all devices."
			   << " If the digital input is turned on, you have to take this into account."
			   << endl;
		return;
	}
	// check for maximum # channels

	//check for consistency between sourcechdevices and sourcechlist per device
	int sourceChListOffset = 0;
	for (int dev = 0; dev < Parameter("DeviceIDs")->NumValues() ; dev++)
	{
		if (Parameter("SourceChList")->NumValues() == 0)
			continue;
            
		int devChs = Parameter("SourceChDevices")(dev);
		vector<int> tmpChList;
		for (int i = sourceChListOffset; i < devChs + sourceChListOffset; i++)
		{
			int curCh = Parameter("SourceChList")(i);

			int maxCh = 19;
			if (tHighSpeed) maxCh = 5;
			if (curCh < 1 || curCh > maxCh)
			{
				bcierr << "SourceChList values must be within the range of 1 to "<< maxCh <<endl;
				return;
			}
			if (find(tmpChList.begin(), tmpChList.end(), curCh) != tmpChList.end())
			{
				bcierr << "SourceChList may not contain duplicate values for an individual device"<<endl;
				return;
			}
			tmpChList.push_back(curCh);
		}
		sourceChListOffset += devChs;
	}
	State("Running");
}

int vAmpADC::getDecimation() const
{
	bool tHighSpeed = Parameter("AcquisitionMode") == 1 || Parameter("AcquisitionMode") == 4;
	float reqFs = Parameter("SamplingRate");
	if (mHighSpeed){
		int dec = int(20000.0/reqFs);
		if (abs(float(dec) - 20000.0/reqFs) < .02)
			return dec;
		else
			return 0;
	}
	else{
		int dec = int(2000.0/reqFs);
		if (abs(float(dec) - 2000.0/reqFs) < .02)
			return dec;
		else
			return 0;
	}

}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the vAmpADC
// **************************************************************************
void vAmpADC::Initialize(const SignalProperties&, const SignalProperties&)
{
	this->Halt();
	mHighSpeed = Parameter("AcquisitionMode") == 1 || Parameter("AcquisitionMode") == 4;
	UINT tNumDevices = FA_ID_INVALID;
	  tNumDevices = faGetCount();
	  if (tNumDevices < 1) {
		  bcierr <<"No vAmp devices were found."<<endl;
	  }
	  if (tNumDevices > MAX_ALLOWED_DEVICES) {
			bcierr << "A maximum of " << MAX_ALLOWED_DEVICES << " devices can be present on the system at a time."<<endl;
			return;     
	  }
	t_faInformation tDeviceInfo[MAX_ALLOWED_DEVICES];
	for (int i = 0; i < tNumDevices; i++){
		faGetInformation(faGetId(i), &(tDeviceInfo[i]));
	}

	int sourceChListOffset = 0;
	mChList.clear();
	mDevList.clear();
	int mDevChList[MAX_ALLOWED_DEVICES];
	for (int dev = 0; dev < Parameter("DeviceIDs")->NumValues() ; dev++)
	{
		for (int i = 0; i < tNumDevices; i++)
			if (tDeviceInfo[i].SerialNumber == Parameter("DeviceIDs")(dev))
				mDevList.push_back(faGetId(i));
        if( tNumDevices == 1 && mDevList.empty() )
        {
          bciout << "Wrong serial # (" << Parameter( "DeviceIDs" )( 0 )
                 << ") specified for single amplifier, using amplifier with serial # "
                 << tDeviceInfo[0].SerialNumber
                 << endl;
          mDevList.push_back( faGetId(0) );
        }

		int devChs = Parameter("SourceChDevices")(dev);
		mDevChList[dev] = devChs;
		if (Parameter("SourceChList")->NumValues() == 0){
			int nChsTmp;
			switch (tDeviceInfo[dev].Model){
				case FA_MODEL_8:
					nChsTmp = FA_MODEL_8_CHANNELS_MAIN + FA_MODEL_8_CHANNELS_AUX;
					break;
				case FA_MODEL_16:
					nChsTmp = FA_MODEL_16_CHANNELS_MAIN + FA_MODEL_16_CHANNELS_AUX;
					break;
				default:
					break;
			}
			if (mHighSpeed)
            	nChsTmp = 5;
			for (int i = 0; i < nChsTmp; i++)
				mChList.push_back(i+sourceChListOffset);
		}
		else
		{
			for (int i = sourceChListOffset; i < devChs + sourceChListOffset; i++)
			{
				int curCh = Parameter("SourceChList")(i);
				mChList.push_back(curCh-1);
			}
		}
		sourceChListOffset += devChs;
	}
	int decimate = 1;
	float fs = Parameter("SamplingRate");
	decimate= getDecimation();
	float hpCorner = 0;
	if (Parameter("HPFilterCorner")->NumValues() == 1)
		hpCorner = Parameter("HPFilterCorner");
		
	mAcquire = new vAmpThread(
						Parameter("SampleBlockSize"),
						fs,
						decimate,
						mChList,
						mDevChList,
						mDevList,
						Parameter("AcquisitionMode"),
						hpCorner/fs);
	if (!mAcquire->ok())
	{
		bcierr << mAcquire->lastErr() << endl;
		delete mAcquire;
		return;
	}

	ResetEvent( mAcquire->acquireEventRead );
	mAcquire->Resume();
}


// **************************************************************************
// Function:   Process
// Purpose:    This function reads from the data buffer that is written to
//              in the acquisition thread Execute function
//
// Parameters: References to input signal (ignored) and output signal
// Returns:    N/A
// **************************************************************************
void vAmpADC::Process( const GenericSignal&, GenericSignal& signal )
{
	if (mAcquire == NULL){
		bcierr << "Invalid acquisition thread! " << endl;
		State("Running") = 0;
		return;
	}
	if (!mAcquire->ok()){
		bcierr << mAcquire->lastErr() << endl;
		State("Running") = 0;
		return;
	}
	if (mAcquire->IsTerminating())
	{
		for (int sample = 0; sample < signal.Elements(); sample++)
			for (int ch = 0; ch < signal.Channels(); ch++)
				signal(ch, sample) = 0;
		ResetEvent( mAcquire->acquireEventRead );
		return;
	}

	
	if (WaitForSingleObject(mAcquire->acquireEventRead, 1000) != WAIT_OBJECT_0)
	{
		for (int sample = 0; sample < signal.Elements(); sample++)
			for (int ch = 0; ch < signal.Channels(); ch++)
				signal(ch, sample) = 0;
		ResetEvent( mAcquire->acquireEventRead );
		return;
	}
	for (int sample = 0; sample < signal.Elements(); sample++){
		for (int ch = 0; ch < signal.Channels(); ch++)
		{
			signal(ch, sample) = mAcquire->ExtractData(ch, sample);
		}
	}


	mAcquire->AdvanceReadBlock();
	ResetEvent( mAcquire->acquireEventRead );

}


// **************************************************************************
// Function:   Halt
// Purpose:    This routine shuts down data acquisition
// Parameters: N/A
// Returns:    N/A
// **************************************************************************

void vAmpADC::Halt()
{
	if (mAcquire != NULL)
	{
		//mAcquire->Suspend();
		mAcquire->Terminate();
		while (!mAcquire->IsTerminated())
			Sleep(10);
		delete mAcquire;
		mAcquire = NULL;
	}

}                              
						
