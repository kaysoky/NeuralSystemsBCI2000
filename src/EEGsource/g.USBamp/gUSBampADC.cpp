/******************************************************************************
 * Program:   EEGsource.EXE                                                   *
 * Module:    gUSBampADC.CPP                                                  *
 * Comment:   BCI2000 Source Module for gUSBamp devices                       *
 * Version:   1.10                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V1.00 - 09/23/2004 - First working version                                 *
 * V1.10 - 11/19/2004 - Filters are only set when changed, can sample != 16 ch*
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "gUSBampADC.h"
#include "gUSBamp.h"

#include "UGenericSignal.h"
#include <stdio.h>
#include <math.h>

using namespace std;

// Register the source class with the framework.
RegisterFilter( gUSBampADC, 1 );

// **************************************************************************
// Function:   gUSBampADC
// Purpose:    The constructor for the gUSBampADC
//             it fills the provided list of parameters and states
//             with the parameters and states it requests from the operator
// Parameters: plist - the list of parameters
//             slist - the list of states
// Returns:    N/A
// **************************************************************************
gUSBampADC::gUSBampADC()
: mFloatOutput( false )
{
 // add all the parameters that this ADC requests to the parameter list
 BEGIN_PARAMETER_DEFINITIONS
   "Source int SoftwareCh=      16 16 1 128 "
       "// number of digitized channels total",
   "Source intlist SoftwareChDevices=  1 16 16 1 16"
       "// number of digitized channels per device",
   "Source int SampleBlockSize= 8 5 1 128 "
       "// number of samples per block",
   "Source string DeviceIDMaster= auto 1 1 16"
       "// deviceID for the device whose SYNC goes to the slaves",
   "Source int SamplingRate=    128 128 1 40000 "
       "// the signal sampling rate",
   "Source int FilterEnabled= 1 1 0 1 "
       "// Enable pass band filter (0=no, 1=yes)",
   "Source float FilterHighPass=   0.1 0.1 0 50 "
       "// high pass filter for pass band",
   "Source float FilterLowPass=    60 60 0 4000 "
       "// low pass filter for pass band",
   "Source int FilterModelOrder= 4 4 1 10 "
       "// filter model order for pass band",
   "Source int FilterType= 1 1 1 2"
       "// filter type for pass band (1=CHEBYSHEV, 2=BUTTERWORTH)",
   "Source int NotchEnabled= 1 1 0 1 "
       "// Enable notch (0=no, 1=yes)",
   "Source float NotchHighPass=   50 50 0 70 "
       "// high pass filter for notch filter",
   "Source float NotchLowPass=    70 70 0 4000 "
       "// low pass filter for notch filter",
   "Source int NotchModelOrder= 2 2 1 10 "
       "// filter model order for notch filter",
   "Source int NotchType= 1 1 1 2"
       "// filter type for pass band (1=CHEBYSHEV, 2=BUTTERWORTH)",
   "Source list DeviceIDs= 1 auto"
       "// list of USBamps to be used (or auto)",
   "Source int SignalType=           0 0 0 1"
        "// numeric type of output signal: "
            " 0: int16,"
            " 1: float32"
            "(enumeration)",
 END_PARAMETER_DEFINITIONS

 // add all states that this ADC requests to the list of states
 // this is just an example (here, we don't really need all these states)
 BEGIN_STATE_DEFINITIONS
   "Running 1 0 0 0",
   "SourceTime 16 2347 0 0",
 END_STATE_DEFINITIONS

 m_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
 pBuffer.resize(0);
 numdevices=0;

 // set the priority of the Source module a little higher
 // (since its output clocks the whole system)
 // this really seems to help against data loss; even under extreme load,
 // data acquisition continues even though the display might be jerky
 SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
}


gUSBampADC::~gUSBampADC()
{
 Halt();
 for (unsigned int dev=0; dev<pBuffer.size(); dev++)
  if (pBuffer.at(dev)) delete [] pBuffer.at(dev);
 CloseHandle(m_hEvent);
}


// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void gUSBampADC::Preflight( const SignalProperties&,
                                       SignalProperties& outSignalProperties ) const
{
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  if( Parameter("SourceChGain")->GetNumValues() != Parameter("SoftwareCh") )
    bcierr << "# elements in SourceChGain has to match total # channels" << endl;
  if( Parameter("SourceChOffset")->GetNumValues() != Parameter("SoftwareCh") )
    bcierr << "# elements in SourceChOffset has to match total # channels" << endl;

  bool goodSourceChGain = true,
       goodSourceChOffset = true;
  for (int ch=0; ch<Parameter("SoftwareCh"); ch++)
   {
   goodSourceChGain = goodSourceChGain && ( Parameter("SourceChGain", ch) > 0 );
   goodSourceChOffset = goodSourceChOffset && ( fabs(Parameter("SourceChOffset", ch)) < 0.0001 );
   }
  if( !goodSourceChGain )
    bcierr << "SourceChGain is not supposed to be zero" << endl;
  if( !goodSourceChOffset )
    bcierr << "SourceChOffset is supposed to be zero" << endl;

  if( Parameter("DeviceIDs")->GetNumValues() != Parameter("SoftwareChDevices")->GetNumValues() )
    bcierr << "# devices has to equal # entries in SoftwareChDevices" << endl;

  int totalnumchannels=0;
  for (unsigned int dev=0; dev<Parameter("DeviceIDs")->GetNumValues(); dev++)
   totalnumchannels += Parameter("SoftwareChDevices", dev);
  if( Parameter("SoftwareCh") != totalnumchannels )
    bcierr << "# total channels has to equal sum of all channels over all devices" << endl;

  bool DeviceIDMaster=false;
  for (unsigned int dev=0; dev<Parameter("DeviceIDs")->GetNumValues(); dev++)
   if (Parameter("DeviceIDs") == Parameter("DeviceIDMaster"))
      DeviceIDMaster=true;
  if( !DeviceIDMaster )
    bcierr << "the MasterDevice has to be one of the DeviceIDs" << endl;

  bool goodBlockSize = true;
  for (unsigned int dev=0; dev<Parameter("DeviceIDs")->GetNumValues(); dev++)
   goodBlockSize = goodBlockSize && ((int)(Parameter("SoftwareChDevices", dev)*Parameter("SampleBlockSize")*sizeof(float))%512 == 0 );
  if( !goodBlockSize )
    bcierr << "currently, the size in bytes of each sample block has to be"
           << " a multiple of 512 (limitation of the driver)"
           << endl;

  //
  // From here down, determine device settings and verify if OK
  //
  // if we have one device ID, and it's set to 'auto' check whether we can proceed in auto mode
  if ((Parameter("DeviceIDs")->GetNumValues() == 1) && (string(Parameter("DeviceIDs"))=="auto"))
  {
    int detectionResult = DetectAutoMode();
    if( detectionResult == -1 )
      bcierr << "Could not detect any amplifier. "
             << "Make sure there is a single gUSBamp amplifier connected to your system, and switched on"
             << endl;
    else if( detectionResult == -2 )
      bcierr << "Too many amplifiers connected. "
             << "In auto-configuring mode, only a single amplifier may be connected"
             << endl;
    else if( detectionResult < 0 )
      bcierr << "Auto-detection of amplifier failed"
             << endl;
  }
  else // if we defined more than one device or not auto mode, try to open and test all devices
     {
     int numdevices=Parameter("DeviceIDs")->GetNumValues();
     // test to open all devices and to set the sampling rate
     for (int dev=0; dev<numdevices; dev++)
      {
      HANDLE hdev = GT_OpenDeviceEx((char *)Parameter("DeviceIDs")->GetValue(dev));
      if (!hdev)
         bcierr << "Could not open device " << string(Parameter("DeviceIDs")->GetValue(dev)) << endl;
      else
         {
         // let's check whether the driver complains if we use a wrong sampling rate
         // according to the documentation, it should
         // it looks like in practise, it does not
         int samplerate=Parameter("SamplingRate");
         if (!GT_SetSampleRate(hdev, samplerate))
         {
           WORD wErrorCode;
           char LastError[512] = "unavailable";
           GT_GetLastError(&wErrorCode, LastError);
           bcierr << "Could not set sampling rate on device " << string(Parameter("DeviceIDs")->GetValue(dev))
                  << " (error details: " << LastError << ")"
                  << endl;
         }
         GT_CloseDevice(&hdev);
         // thus, let's do the check that the driver is supposed to do again here
         if ((samplerate != 32) &&
             (samplerate != 64) &&
             (samplerate != 128) &&
             (samplerate != 256) &&
             (samplerate != 512) &&
             (samplerate != 600) &&
             (samplerate != 2400) &&
             (samplerate != 4800))
            bciout << "Warning: Sampling rate does not seem to be supported. Be aware of your limited Karma!" << endl;
         }
      }
     }

  // check pass band filter settings
  if (Parameter("FilterEnabled") == 1)
     if (DetermineFilterNumber() == -1)
        bcierr << "Could not find appropriate pass band filter in gUSBamp. Use gUSBampgetinfo tool." << endl;

  // check notch filter settings
  if (Parameter("NotchEnabled") == 1)
     if (DetermineNotchNumber() == -1)
        bcierr << "Could not find appropriate notch filter in gUSBamp. Use gUSBampgetinfo tool." << endl;

  // Resource availability checks.
  /* The random source does not depend on external resources. */

  // Input signal checks.
  /* The input signal will be ignored. */

  // Requested output signal properties.
  SignalType signalType = SignalType::int16;
  if( Parameter( "SignalType" ) == 1 )
    signalType = SignalType::float32;
  outSignalProperties = SignalProperties(
       Parameter( "SoftwareCh" ), Parameter( "SampleBlockSize" ), signalType );
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the gUSBampADC
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop (fMain->MainDataAcqLoop())
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void gUSBampADC::Initialize()
{
bool    ret;
int     filternumber, notchnumber;
GND     CommonGround;
REF     CommonReference;
bool    autoconfigure;

 // let's buffer up to 5 times as much data as we'll collect (using GetData)
 // this way, it is possible to be too slow to pick up data but still not lose data
 // (obviously, we should never be too slow in the first place)
 // this doesn't seem to work yet
 // int numberofbuffers=3;

 int samplingrate=Parameter("SamplingRate");
 MasterDeviceID=string(Parameter("DeviceIDMaster"));

 // if we set DeviceIDs to auto and we only have one device, we can autoconfigure
 if ((Parameter("DeviceIDs")->GetNumValues() == 1) && (strcmp((const char *)Parameter("DeviceIDs"), "auto") == 0))
    autoconfigure=true;
 else // otherwise configure the usual way (i.e., using serial numbers)
    autoconfigure=false;

 for (unsigned int dev=0; dev<pBuffer.size(); dev++)
  if (pBuffer.at(dev)) delete [] pBuffer.at(dev);

 // determine the LSB for each channel
 LSB.resize(Parameter("SourceChGain")->GetNumValues());
 for (unsigned int ch=0; ch<LSB.size(); ch++)
  LSB.at(ch)=Parameter("SourceChGain", ch);

 // determine the filter and notch number from the one selected
 //
 filternumber=notchnumber=-1;
 if (Parameter("FilterEnabled") == 1)
    {
    filternumber=DetermineFilterNumber();
    }

 // get notch filter settings if notch is turned on
 if (Parameter("NotchEnabled") == 1)
    {
    notchnumber=DetermineNotchNumber();
    }

 // set the GND structure; connect the GNDs on all four blocks to common ground
 CommonGround.GND1=true;
 CommonGround.GND2=true;
 CommonGround.GND3=true;
 CommonGround.GND4=true;
 // the same with the reference
 CommonReference.ref1=true;
 CommonReference.ref2=true;
 CommonReference.ref3=true;
 CommonReference.ref4=true;

 //
 // at the moment, we CANNOT determine whether we've lost some data
 // thus, we set the timeout to be small, e.g., 1.5 times the size of
 // one sample block and simply give a warning
 // this is not perfect but I simply can't do it better at the moment
 timeoutms=(int)((Parameter("SampleBlockSize")/Parameter("SamplingRate"))*1000*1.5);

 // determine the number of devices and allocate the buffers accordingly
 numdevices=Parameter("DeviceIDs")->GetNumValues();
 DeviceIDs.resize(numdevices);
 hdev.resize(numdevices);
 pBuffer.resize(numdevices);
 numchans.resize(numdevices);
 iBytesperScan.resize(numdevices);
 buffersize.resize(numdevices);
 // configure all devices
 for (int dev=0; dev<numdevices; dev++)
  {
  numchans.at(dev)=Parameter("SoftwareChDevices", dev);
  iBytesperScan.at(dev)=numchans.at(dev)*sizeof(float);
  int pointsinbuffer=Parameter("SampleBlockSize")*numchans.at(dev);
  buffersize.at(dev)=pointsinbuffer*sizeof(float)+HEADER_SIZE;    // buffer size in bytes
  UCHAR *channels=new UCHAR[numchans.at(dev)];
  pBuffer.at(dev)=new BYTE[buffersize.at(dev)];
  // pBuffer.at(dev)=new BYTE[pointsinbuffer*sizeof(float)*numberofbuffers+HEADER_SIZE];  // if we have one buffer, it equals buffersize.at(dev)
  // determine the serial number either automatically (if autoconfigure mode) or from DeviceID parameter
  if (autoconfigure)
     {
     char serialnr[16];
     HANDLE hdev=GT_OpenDevice(DetectAutoMode());
     if( !hdev ) // better safe than sorry ;-)
       bcierr << "Could not open Amplifier device" << endl;
     GT_GetSerial(hdev, (LPSTR)serialnr, 16);     // 16 according to documentation
     GT_CloseDevice(&hdev);
     DeviceIDs.at(dev)=string(serialnr);
     MasterDeviceID=DeviceIDs.at(dev);
     }
  else
     DeviceIDs.at(dev)=string(Parameter("DeviceIDs", dev));
  hdev.at(dev) = GT_OpenDeviceEx((char *)DeviceIDs.at(dev).c_str());
  if( !hdev.at( dev ) ) // better safe than sorry ;-)
    bcierr << "Could not open Amplifier with ID "
           << DeviceIDs.at( dev )
           << endl;

  ret=GT_SetBufferSize(hdev.at(dev), Parameter( "SampleBlockSize" ) );
  // set all devices to slave except the one master
  // externally, the master needs to have its SYNC OUT wired to the SYNC IN
  // of the first slave (whos SYNC OUT is connected to the next slave's SYNC IN)
  if (DeviceIDs.at(dev) == MasterDeviceID)
     ret=GT_SetSlave(hdev.at(dev), false);
  else
     ret=GT_SetSlave(hdev.at(dev), true);
  ret=GT_SetGround(hdev.at(dev), CommonGround);         // connect the grounds from all four blocks on each device to common ground
  ret=GT_SetReference(hdev.at(dev), CommonReference);   // the same for the reference
  ret=GT_SetSampleRate(hdev.at(dev), samplingrate);
  // ret=GT_EnableSC(hdev.at(dev), true);  // with the short cut mode, a TTL pulse on the SC connector puts the inputs on internal GND (we don't need this?)

  // here, we could check for whether or not the filter settings in the USBamp match what we want; if so; no need to change
  // this might take a long time
  // in the current implementation, we set the same filter and notch on all channels
  // (otherwise, would be many parameters)
  // because it takes so long, set filters only when they have been changed
  static oldfilternumber=-999, oldnotchnumber=-999;
  for (int ch=0; ch<numchans.at(dev); ch++)
   {
   if (oldfilternumber != filternumber) GT_SetBandPass(hdev.at(dev), ch+1, filternumber);
   if (oldnotchnumber != notchnumber) GT_SetNotch(hdev.at(dev), ch+1, notchnumber);
   }
  oldfilternumber=filternumber;
  oldnotchnumber=notchnumber;

  // set the channel list for sampling
  for (int ch=0; ch<numchans.at(dev); ch++)
   channels[ch] = ch+1;

  ret=GT_SetChannels(hdev.at(dev), channels, (UCHAR)numchans.at(dev));
  delete [] channels;
  }

 // once the configuration is all done (which might take a while), start them up
 // now, start the slaves first
 for (int dev=0; dev<numdevices; dev++)
  if (DeviceIDs.at(dev) != MasterDeviceID)
     ret=GT_Start(hdev.at(dev));
 // at last, start the Master (so that the slaves are all ready when the Master starts sampling)
 for (int dev=0; dev<numdevices; dev++)
  if (DeviceIDs.at(dev) == MasterDeviceID)
     ret=GT_Start(hdev.at(dev));

  mFloatOutput = ( Parameter( "SignalType" ) == 1 );
}


// **************************************************************************
// Function:   Process
// Purpose:    This function is called within fMain->MainDataAcqLoop()
//             it fills the already initialized array RawEEG with values
//             and DOES NOT RETURN, UNTIL ALL DATA IS ACQUIRED
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void gUSBampADC::Process( const GenericSignal*, GenericSignal* signal )
{
 OVERLAPPED ov;
 DWORD dwBytesReceived = 0;
 DWORD dwOVret;

 ov.hEvent = m_hEvent;
 ov.Offset = 0;
 ov.OffsetHigh = 0;

 // iterate through all devices
 int cur_ch=0;
 float *cur_sampleptr, cur_sample;
 for (unsigned int dev=0; dev<hdev.size(); dev++)
  {
  bool ret = GT_GetData(hdev.at(dev), pBuffer.at(dev), buffersize.at(dev), &ov);
  // here one should implement a circular buffer running in a separate thread
  // to ensure that data are retrieved quickly enough
  // could use the Matlab 7.0 circular buffer: \MATLAB7\toolbox\daq\daq\src\include\cirbuf.h
  // while (!HasOverlappedIoCompleted(&ov))
  //  Sleep(0);
  // we can't directly determine at the moment whether we lost some data
  // simply have a timeout that's 1.5 times one sample block and notify the operator that we were too slow :-(
  dwOVret = WaitForSingleObject(m_hEvent, timeoutms);
  if (dwOVret == WAIT_TIMEOUT)
     {
     bciout << "Signals lost during acquisition. "
            << "Make sure that the g.USBamp is attached to a USB 2.0 port, or "
            << "set SampleBlockSize larger !!"
            << endl;
     // ResetEvent(m_hEvent);
     // GT_ResetTransfer(hdev.at(dev));
     // throw;
     }
  GetOverlappedResult(hdev.at(dev), &ov, &dwBytesReceived, FALSE);
  if( mFloatOutput )
  {
    float* data = reinterpret_cast<float*>( pBuffer[ dev ] + HEADER_SIZE );
    for( size_t sample = 0; sample < signal->Elements(); ++sample )
      for( int channel = 0; channel < numchans[ dev ]; ++channel )
        ( *signal )( cur_ch + channel, sample ) = *data++ / LSB[ cur_ch + channel ];
  }
  else
  {
    for (size_t sample=0; sample<signal->Elements(); sample++)
     {
     for (int channel=0; channel<numchans.at(dev); channel++)
      {
      cur_sampleptr=(float *)(pBuffer.at(dev) + HEADER_SIZE + (iBytesperScan.at(dev))*sample + channel*4);
      cur_sample=*cur_sampleptr/LSB.at(cur_ch+channel);  // multiplied with 1 over SourceChGain
                                                         // with this scheme, the investigator can define what the resolution of the target signal is
      if (cur_sample > 32767) cur_sample=32767;
      if (cur_sample < -32767) cur_sample=-32767;
      signal->SetValue(cur_ch+channel, sample, (short)(cur_sample+0.5));
      }
     }
  }
  cur_ch += numchans.at(dev);
 }
}


// **************************************************************************
// Function:   Halt
// Purpose:    This routine shuts down data acquisition
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void gUSBampADC::Halt()
{
 // stop the master first
 for (unsigned int dev=0; dev<hdev.size(); dev++)
  {
  if ((hdev.at(dev)) && (DeviceIDs.at(dev) == MasterDeviceID))
     {
     GT_Stop(hdev.at(dev));
     GT_ResetTransfer(hdev.at(dev));
     GT_CloseDevice(&(hdev.at(dev)));
     }
  }
 // finally, stop the slaves
 for (unsigned int dev=0; dev<hdev.size(); dev++)
  {
  if ((hdev.at(dev)) && (DeviceIDs.at(dev) != MasterDeviceID))
     {
     GT_Stop(hdev.at(dev));
     GT_ResetTransfer(hdev.at(dev));
     GT_CloseDevice(&(hdev.at(dev)));
     }
  }
}

// **************************************************************************
// Function:   DetectAutoMode
// Purpose:    This function determines whether the system can be configured automatically.
//             This is the case when there is exactly one amplifier connected.
// Parameters: N/A
// Returns:    0..15 ... USB ID of the one connected amplifier
//             -1 ... no amplifier detected
//             -2 ... more than one amplifier detected
// **************************************************************************
int gUSBampADC::DetectAutoMode() const
{
 int numdetected=0, USBport=-1;

 for (int cur_USBport=0; cur_USBport<16; cur_USBport++)
  {
  HANDLE hdev = GT_OpenDevice(cur_USBport);
  if (hdev)
     {
     numdetected++;
     GT_CloseDevice(&hdev);
     USBport=cur_USBport;
     }
  }

 if (numdetected > 1)  return(-2);
 if (numdetected == 0) return(-1);
 return(USBport);
}


// **************************************************************************
// Function:   DetermineFilterNumber
// Purpose:    This routine determines the pass band filter number that
//             a particular parameter setting corresponds to
// Parameters: N/A
// Returns:    >=0: pass band filter number
//             -1 filter number not found
// **************************************************************************
int gUSBampADC::DetermineFilterNumber() const
{
int     nof;
FILT    *filt;

 int samplingrate=Parameter("SamplingRate");
 int filternumber = -1;

 GT_GetNumberOfFilter(&nof);
 filt = new _FILT[nof];
 for (int no_filt=0; no_filt<nof; no_filt++)
  {
  GT_GetFilterSpec(filt);
  if ((fabs(filt[no_filt].fu-Parameter("FilterHighPass")) < 0.0001) &&
      (fabs(filt[no_filt].fo-Parameter("FilterLowPass")) < 0.0001) &&
      (filt[no_filt].fs == samplingrate) &&
      (filt[no_filt].order == Parameter("FilterModelOrder")) &&
      (filt[no_filt].type == Parameter("FilterType")))
     filternumber=no_filt;
  }
 delete[] filt;

 return(filternumber);
}


// **************************************************************************
// Function:   DetermineNotchNumber
// Purpose:    This routine determines the notch filter number that
//             a particular parameter setting corresponds to
// Parameters: N/A
// Returns:    >=0: notch filter number
//             -1 filter number not found
// **************************************************************************
int gUSBampADC::DetermineNotchNumber() const
{
int     nof;
FILT    *filt;

 int samplingrate=Parameter("SamplingRate");
 int notchnumber = -1;

 GT_GetNumberOfNotch(&nof);
 filt = new _FILT[nof];
 for (int no_filt=0; no_filt<nof; no_filt++)
  {
  GT_GetNotchSpec(filt);
  if ((fabs(filt[no_filt].fu-Parameter("NotchHighPass")) < 0.0001) &&
      (fabs(filt[no_filt].fo-Parameter("NotchLowPass")) < 0.0001) &&
      (filt[no_filt].fs == samplingrate) &&
      (filt[no_filt].order == Parameter("NotchModelOrder")) &&
      (filt[no_filt].type == Parameter("NotchType")))
     notchnumber=no_filt;
  }
 delete[] filt;

 return(notchnumber);
}

