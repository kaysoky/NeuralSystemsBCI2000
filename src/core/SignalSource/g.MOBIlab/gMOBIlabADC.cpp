////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org
// Description: BCI2000 Source Module for gMOBIlab devices.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "gMOBIlabADC.h"

using namespace std;

// Register the source class with the framework.
RegisterFilter( gMOBIlabADC, 1 );

// **************************************************************************
// Function:   gMOBIlabADC
// Purpose:    The constructor for the gMOBIlabADC
// **************************************************************************
gMOBIlabADC::gMOBIlabADC()
{
 // add all the parameters that this ADC requests to the parameter list
 BEGIN_PARAMETER_DEFINITIONS
   "Source string COMport=      COM2: COM2: a z "
       "// COMport for MOBIlab",
   "Source int SourceCh=      16 16 1 128 "
       "// number of digitized channels total",
   "Source int SampleBlockSize= 8 5 1 128 "
       "// number of samples per block",
   "Source int SamplingRate=    256 256 1 40000 "
       "// the signal sampling rate",
 END_PARAMETER_DEFINITIONS

 buffer=NULL;
 hDev=NULL;

 // add all states that this ADC requests to the list of states
 // this is just an example (here, we don't really need all these states)
 BEGIN_STATE_DEFINITIONS
   "Running 1 0 0 0",
   "SourceTime 16 2347 0 0",
 END_STATE_DEFINITIONS

 // set the priority of the Source module a little higher
 // (since its output clocks the whole system)
 // this really seems to help against data loss; even under extreme load,
 // data acquisition continues even though the display might be jerky
 SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
}


gMOBIlabADC::~gMOBIlabADC()
{
 Halt();
}


// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void gMOBIlabADC::Preflight( const SignalProperties&,
                                       SignalProperties& outSignalProperties ) const
{
int sampleblocksize=Parameter("SampleBlockSize");
int numchans=Parameter("SourceCh");
int numsamplesperscan=numchans*sampleblocksize;
bool ret;

  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  if ( Parameter("SamplingRate") != 256 )
     bcierr << "MOBIlab sampling rate is fixed at 256 Hz. Change SamplingRate parameter to 256." << endl;
  if ( Parameter("SourceCh") < 1 )
     bcierr << "Number of channels (SourceCh) has to be at least 1." << endl;
  if ( Parameter("SourceCh") > 9 )
     bcierr << "Number of channels (SourceCh) cannot be more than 9." << endl;

  // buffersize can't be > 1024
  if ( numsamplesperscan*sizeof(short) > 1024 )
     bcierr << "Buffer size cannot be larger than 1024. Please decrease the SourceCh or SampleBlocksize." << endl;

  string COMport = Parameter("COMport");

  HANDLE hDev = GT_OpenDevice((LPSTR)COMport.c_str());
  if (hDev == NULL)
     bcierr << "Could not open device at COM port " << COMport << endl;

  _AIN ain;
  _DIO dio;

  ain.ain1 = true;
  ain.ain2 = true;
  ain.ain3 = true;
  ain.ain4 = true;
  ain.ain5 = true;
  ain.ain6 = true;
  ain.ain7 = true;
  ain.ain8 = true;
  dio.scan = true;
  dio.dio1_direction = true; // set dio1 to input
  dio.dio2_direction = true; // set dio2 to input

  // try to initialize the device with all channels
  ret = GT_InitChannels(hDev, ain, dio); // init analog channels and digital lines on g.MOBIlab
  if (!ret) bcierr << "Could not initialize device at COM port " << COMport << endl;

  GT_CloseDevice(hDev);

  // Resource availability checks.
  /* The random source does not depend on external resources. */

  // Input signal checks.
  /* The input signal will be ignored. */

  // Requested output signal properties.
  outSignalProperties = SignalProperties(
       Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), SignalType::int16 );
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the gMOBIlabADC
//             It is called each time parameters have been changed
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void gMOBIlabADC::Initialize(const SignalProperties&, const SignalProperties&)
{
int sampleblocksize=Parameter("SampleBlockSize");
numchans=Parameter("SourceCh");
int numsamplesperscan=numchans*sampleblocksize;
bool ret;

bufsize=numsamplesperscan*sizeof(short);
string COMport = Parameter("COMport");

if (buffer) delete [] buffer;
buffer=new short[bufsize];
hEvent = NULL;

hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
ov.hEvent = hEvent;
ov.Offset = 0;
ov.OffsetHigh = 0;

myBuffer.validPoints = 0;
myBuffer.size = bufsize;
myBuffer.pBuffer = buffer;

hDev = GT_OpenDevice((LPSTR)COMport.c_str());
if (hDev == NULL)
   {
   bcierr << "Could not open device" << endl;
   return;
   }

_AIN ain;
_DIO dio;

ain.ain1 = false;
ain.ain2 = false;
ain.ain3 = false;
ain.ain4 = false;
ain.ain5 = false;
ain.ain6 = false;
ain.ain7 = false;
ain.ain8 = false;
dio.scan = false;

if (numchans > 0) ain.ain1 = true; // scan
if (numchans > 1) ain.ain2 = true; // scan
if (numchans > 2) ain.ain3 = true; // scan
if (numchans > 3) ain.ain4 = true; // scan
if (numchans > 4) ain.ain5 = true; // scan
if (numchans > 5) ain.ain6 = true; // scan
if (numchans > 6) ain.ain7 = true; // scan
if (numchans > 7) ain.ain8 = true; // scan
if (numchans > 8) dio.scan = true; // scan digital lines

dio.dio1_direction = true; // set dio1 to input
dio.dio2_direction = true; // set dio2 to input

ret = GT_InitChannels(hDev, ain, dio); // init analog channels and digital lines on g.MOBIlab
if (!ret)
   {
   bcierr << "Could not initialize device" << endl;
   return;
   }

ret=GT_StartAcquisition(hDev);
if (!ret)
   bcierr << "Could not start data acquisition" << endl;
}


// **************************************************************************
// Function:   Process
// Purpose:    This function is called within the data acquisition loop
//             it fills its output signal with values
//             and DOES NOT RETURN, UNTIL ALL DATA IS ACQUIRED
// Parameters: References to input signal (ignored) and output signal
// Returns:    N/A
// **************************************************************************
void gMOBIlabADC::Process( const GenericSignal&, GenericSignal& signal )
{
bool  ret;
DWORD dwBytesReceived;

 int totalbytesreceived=0;
 // myBuffer.validPoints=0;
 while (totalbytesreceived < (int)myBuffer.size)
  {
  myBuffer.pBuffer = (short *)((unsigned long)buffer+(unsigned long)totalbytesreceived);
  myBuffer.size = bufsize-totalbytesreceived;
  ret = GT_GetData(hDev, &myBuffer, &ov); // extract data from driver
  if ( !ret )
     {
     bcierr << "Unexpected fatal error on GT_GetData()" << endl;
     return;
     }
  WaitForSingleObject(hEvent, INFINITE);
  GetOverlappedResult(hDev, &ov, &dwBytesReceived, FALSE);
  myBuffer.validPoints += dwBytesReceived / sizeof(short);
  totalbytesreceived += dwBytesReceived;
  }

 short *cur_sample;
 for (int sample=0; sample<signal.Elements(); sample++)
  {
  for (int channel=0; channel<numchans; channel++)
   {
   cur_sample=(short *)((char *)buffer + numchans*sizeof(short)*sample + channel*sizeof(short));
   signal(channel, sample) = *cur_sample;
   }
  }
}


// **************************************************************************
// Function:   Halt
// Purpose:    This routine shuts down data acquisition
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void gMOBIlabADC::Halt()
{
 if (hDev)
    {
    GT_StopAcquisition(hDev);
    GT_CloseDevice(hDev);
    hDev = NULL;
    }

 if (buffer)
    {
    delete [] buffer;
    buffer=NULL;
    }
}

