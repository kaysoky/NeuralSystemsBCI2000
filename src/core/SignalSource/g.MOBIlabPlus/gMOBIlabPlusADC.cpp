////////////////////////////////////////////////////////////////////////////////
// $Id: gMOBIlabBTADC.cpp 1542 2007-09-14 18:06:49Z gschalk $
// Author: jawilson@cae.wisc.edu
// Description: BCI2000 Source Module for gMOBIlab devices.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
// This is the ADC module for the gMOBIlab bluetooth. It is based
// on the original gMOBIlab module by Gerwin Schalk and Juergen Mellinger,
// and is modified by Adam Wilson
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "gMOBIlabPlusADC.h"

using namespace std;

// Register the source class with the framework.
RegisterFilter( gMOBIlabPlusADC, 1 );

// **************************************************************************
// Function:   gMOBIlabPlusADC
// Purpose:    The constructor for the gMOBIlabPlusADC
// **************************************************************************
gMOBIlabPlusADC::gMOBIlabPlusADC()
{
 // add all the parameters that this ADC requests to the parameter list
 BEGIN_PARAMETER_DEFINITIONS
   "Source string COMport=      COM3: COM2: a z "
       "// COMport for MOBIlab",
   "Source int SourceCh=      8 16 1 16 "
       "// number of digitized channels total",
   "Source int SampleBlockSize= 8 5 1 128 "
       "// number of samples per block",
   "Source int SamplingRate=    256 256 1 40000 "
       "// the signal sampling rate",
   "Source int InfoMode= 0 0 0 1"
        "// display information about the g.MOBIlabPlus",
   "Source int DigitalEnable= 0 0 0 1 "
        "// read digital inputs 1-8 as channels (boolean)",
   "Source int DigitalOutBlock= 0 0 0 1 "
        "//pulse digital output 7 during data reads (boolean)",
 END_PARAMETER_DEFINITIONS
 /* perhaps this can be added later? Does it make sense to stream data to an
   SD card in a real-time BCI system?
   "Source int EnableSDcard= 0 0 0 1 "
        "// enables streaming to an inserted SD card (boolean)",
   "Source string SDFileName= % % % % "
        "// file name for SD file; blank uses default BCI2000 filename (inputfile)" */

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


gMOBIlabPlusADC::~gMOBIlabPlusADC()
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
void gMOBIlabPlusADC::Preflight( const SignalProperties&,
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
  if ( Parameter("SourceCh") > 8 && Parameter("DigitalEnable") == 0 )
     bcierr << "Number of channels (SourceCh) cannot be more than 8 when digital inputs are not used." << endl;
  if ( Parameter("SourceCh") != 16 && Parameter("DigitalEnable") == 1 )
     bcierr << "Number of channels (SourceCh) must equal 16 when digital inputs are used (8 analog + 8 digital channels)." << endl;
  if (Parameter("DigitalEnable") == 0 && Parameter("DigitalOutBlock") == 1)
    bcierr << "DigitalEnable must be checked to use the DigitalOutBlock."<<endl;
    
  // buffersize can't be > 1024
  if ( numsamplesperscan*sizeof(short) > 1024 )
     bcierr << "Buffer size cannot be larger than 1024. Please decrease the SourceCh or SampleBlocksize." << endl;

  // Requested output signal properties.
  outSignalProperties = SignalProperties(
       Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), SignalType::int16 );
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the gMOBIlabPlusADC
//             It is called each time parameters have been changed
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void gMOBIlabPlusADC::Initialize(const SignalProperties&, const SignalProperties&)
{
    int sampleblocksize=Parameter("SampleBlockSize");
    numAChans = numChans=Parameter("SourceCh");
    numDChans = 0;
    if (Parameter("DigitalEnable") == 1)
    {
        numAChans = 8;
        numDChans = 8;
        numChans = 9;
    }
    int numsamplesperscan=numChans*sampleblocksize;
    bool ret;

    Halt();
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
       //there seems to be a bug (or feature...?) where if you open the device
       //after it is already opened, it actually disconnects the bluetooth connection
       //and it needs to be reconnected. This tests for this bug, since it will reconnect
       //on the 2nd try if possible; if there is another problem, it gives the error and returns
       hDev = GT_OpenDevice((LPSTR)COMport.c_str());
       if (hDev == NULL)
       {
         UINT lastErr;
         GT_GetLastError(&lastErr);
         _ERRSTR errorStr;
         GT_TranslateErrorCode(&errorStr,lastErr);
         bcierr << "Could not open device: " << errorStr.Error<< endl;
         return;
       }
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

    if (numAChans > 0) ain.ain1 = true; // scan
    if (numAChans > 1) ain.ain2 = true; // scan
    if (numAChans > 2) ain.ain3 = true; // scan
    if (numAChans > 3) ain.ain4 = true; // scan
    if (numAChans > 4) ain.ain5 = true; // scan
    if (numAChans > 5) ain.ain6 = true; // scan
    if (numAChans > 6) ain.ain7 = true; // scan
    if (numAChans > 7) ain.ain8 = true; // scan

    bool useDigInputs = (int)Parameter("DigitalEnable");
    dio.dio1_enable = useDigInputs; // set dio1 to input
    dio.dio2_enable = useDigInputs; // set dio2 to input
    dio.dio3_enable = useDigInputs;
    dio.dio4_enable = useDigInputs;
    dio.dio5_enable = useDigInputs;
    dio.dio6_enable = useDigInputs;
    dio.dio7_enable = useDigInputs;
    dio.dio8_enable = useDigInputs;

    dio.dio4_direction = true;
    dio.dio5_direction = true;
    dio.dio6_direction = true;
    dio.dio7_direction = true;
    mEnableDigOut = false;
    if (Parameter("DigitalOutBlock") == 1)
    {
        dio.dio7_direction = false;   //set to digital output
        mEnableDigOut = true;
        GT_SetDigitalOut(hDev, 0x11);
    }

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
void gMOBIlabPlusADC::Process( const GenericSignal&, GenericSignal& signal )
{
    bool  ret;
    DWORD dwBytesReceived;

    int totalbytesreceived=0;
    // myBuffer.validPoints=0;
    if (mEnableDigOut)
        GT_SetDigitalOut(hDev, 0x10);
        
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

    short *cur_sample, dig_sample;
    for (int sample=0; sample<signal.Elements(); sample++)
    {
        for (int channel=0; channel<numChans; channel++)
        {
            cur_sample=(short *)((char *)buffer + numChans*sizeof(short)*sample + channel*sizeof(short));
            if (channel < 8)
            {
                signal(channel, sample) = *cur_sample;
                continue;
            }
            else 
            {
                //the digital lines are stored in a single sample, with the values in each bit
                // the order is (MSB) 8 7 6 5 2 4 3 1 (LSB)
                (*cur_sample) & 0x0001 ? signal(8, sample) =  100 : signal(8, sample)  = 0;
                (*cur_sample) & 0x0002 ? signal(10, sample) = 100 : signal(10, sample) = 0;
                (*cur_sample) & 0x0004 ? signal(11, sample) = 100 : signal(11, sample) = 0;
                (*cur_sample) & 0x0008 ? signal(9, sample) =  100 : signal(9, sample)  = 0;
                (*cur_sample) & 0x0010 ? signal(12, sample) = 100 : signal(12, sample) = 0;
                (*cur_sample) & 0x0020 ? signal(13, sample) = 100 : signal(13, sample) = 0;
                (*cur_sample) & 0x0040 ? signal(14, sample) = 100 : signal(14, sample) = 0;
                (*cur_sample) & 0x0080 ? signal(15, sample) = 100 : signal(15, sample) = 0;
            }
        }
    }
    if (mEnableDigOut)
        GT_SetDigitalOut(hDev, 0x11);
}


// **************************************************************************
// Function:   Halt
// Purpose:    This routine shuts down data acquisition
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void gMOBIlabPlusADC::Halt()
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

