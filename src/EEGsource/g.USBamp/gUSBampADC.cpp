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
#include <assert.h>

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
{
 // add all the parameters that this ADC requests to the parameter list
 BEGIN_PARAMETER_DEFINITIONS
   "Source int SoftwareCh=      16 16 1 128 "
       "// number of digitized channels total",
   "Source intlist SoftwareChDevices=  1 16 16 1 16"
       "// number of digitized channels per device",
   "Source int SampleBlockSize= 8 5 1 128 "
       "// number of samples per block",
   "Source int DeviceIDMaster= 1 1 1 16"
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
   "Source intlist DeviceIDs= 1 1"
       "// list of USBamps to be used ",
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
// Constants.
const size_t signalDepth = 2;

  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  // # elements in SourceChGain has to match total # channels
  PreflightCondition( Parameter("SourceChGain")->GetNumValues() == Parameter("SoftwareCh") );
  // # elements in SourceChOffset has to match total # channels
  PreflightCondition( Parameter("SourceChOffset")->GetNumValues() == Parameter("SoftwareCh") );

  // SourceChGain is not supposed to be zero
  // SourceChOffset is supposed to be zero
  for (int ch=0; ch<Parameter("SoftwareCh"); ch++)
   {
   PreflightCondition( Parameter("SourceChGain", ch) > 0 );
   PreflightCondition( abs(Parameter("SourceChOffset", ch)) < 0.0001 );
   }

  // # devices has to equal # entries in SoftwareChDevices
  PreflightCondition( Parameter("DeviceIDs")->GetNumValues() == Parameter("SoftwareChDevices")->GetNumValues() );

  // # total channels has to equal sum of all channels over all devices
  int totalnumchannels=0;
  for (unsigned int dev=0; dev<Parameter("DeviceIDs")->GetNumValues(); dev++)
   totalnumchannels += Parameter("SoftwareChDevices", dev);
  PreflightCondition( Parameter("SoftwareCh") == totalnumchannels );

  // the MasterDevice has to be one of the DeviceIDs
  bool DeviceIDMaster=false;
  for (unsigned int dev=0; dev<Parameter("DeviceIDs")->GetNumValues(); dev++)
   if (Parameter("DeviceIDs") == Parameter("DeviceIDMaster"))
      DeviceIDMaster=true;
  PreflightCondition( DeviceIDMaster == true );

  // currently, the size in bytes of each sample block has to be a multiple of 512
  // (limitation of the driver)
  for (unsigned int dev=0; dev<Parameter("DeviceIDs")->GetNumValues(); dev++)
   PreflightCondition((int)(Parameter("SoftwareChDevices", dev)*Parameter("SampleBlockSize")*sizeof(float))%512 == 0 );

  // Resource availability checks.
  /* The random source does not depend on external resources. */

  // Input signal checks.
  /* The input signal will be ignored. */

  // Requested output signal properties.
  outSignalProperties = SignalProperties(
       Parameter( "SoftwareCh" ), Parameter( "SampleBlockSize" ), signalDepth );
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
int     nof;
FILT    *filt;
bool    ret;
int     filternumber, notchnumber;
GND     CommonGround;
REF     CommonReference;

 // let's buffer up to 5 times as much data as we'll collect (using GetData)
 // this way, it is possible to be too slow to pick up data but still not lose data
 // (obviously, we should never be too slow in the first place)
 // this doesn't seem to work yet
 // int numberofbuffers=3;

 int samplingrate=Parameter("SamplingRate");
 MasterDeviceID=string(Parameter("DeviceIDMaster"));

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
    delete filt;
    if (filternumber == -1) bciout << "Could not find appropriate pass band filter in gUSBamp. Use gUSBampgetinfo tool." << endl;
    }

 // get notch filter settings if notch is turned on
 if (Parameter("NotchEnabled") == 1)
    {
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
    delete filt;
    if (notchnumber == -1) bciout << "Could not find appropriate notch filter in gUSBamp. Use gUSBampgetinfo tool." << endl;
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
 // thus, we set the timeout to be small, i.e., 1.5 times the size of
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
  DeviceIDs.at(dev)=string(Parameter("DeviceIDs", dev));
  hdev.at(dev) = GT_OpenDeviceEx((char *)DeviceIDs.at(dev).c_str());
  if (!hdev.at(dev))
     {
     bcierr << "Could not open device " << DeviceIDs.at(dev) << endl;
     break;
     }

  ret=GT_SetBufferSize(hdev.at(dev), buffersize.at(dev));
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
     bciout << "Signals lost during acquisition. Set SampleBlockSize larger !!" << endl;
     // ResetEvent(m_hEvent);
     // GT_ResetTransfer(hdev.at(dev));
     // throw;
     }
  GetOverlappedResult(hdev.at(dev), &ov, &dwBytesReceived, FALSE);
  for (int sample=0; sample<signal->MaxElements(); sample++)
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
 // int tintifax=State("Running");

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





