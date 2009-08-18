////////////////////////////////////////////////////////////////////////////////
// $Id: gMOBIlabBTADC.cpp 1542 2007-09-14 18:06:49Z gschalk $
// Author: jawilson@cae.wisc.edu
// Description: BCI2000 Source Module for gMOBIlab devices.
//   This is the ADC module for the gMOBIlab bluetooth. It is based
//   on the original gMOBIlab module by Gerwin Schalk and Juergen Mellinger,
//   and is modified by Adam Wilson
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "gMOBIlabPlusADC.h"
#include "gMOBIlabDriver.h"

using namespace std;

// Register the source class with the framework.
RegisterFilter( gMOBIlabPlusADC, 1 );

// **************************************************************************
// Function:   gMOBIlabPlusADC
// Purpose:    The constructor for the gMOBIlabPlusADC.
// **************************************************************************
gMOBIlabPlusADC::gMOBIlabPlusADC()
: mDev( NULL ),
  mpAcquisitionThread( NULL )
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
   "Source int TestMode= 0 0 0 1"
   		"// generate a test signal (boolean)",
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
}


gMOBIlabPlusADC::~gMOBIlabPlusADC()
{
  Halt();
}


// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally required properties
//             for the output signal; checks whether resources are available.
// Parameters: References to input and output signal properties.
// Returns:    N/A
// **************************************************************************
void gMOBIlabPlusADC::Preflight( const SignalProperties&,
                                       SignalProperties& Output ) const
{
  int sourceCh = Parameter( "SourceCh" ),
      sampleBlockSize = Parameter( "SampleBlockSize" );
      
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

  // Requested output signal properties.
  Output = SignalProperties( sourceCh, sampleBlockSize, SignalType::int16 );
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the gMOBIlabPlusADC.
//             It is called each time parameters have been changed.
// Parameters: References to input and output signal properties.
// Returns:    N/A
// **************************************************************************
void gMOBIlabPlusADC::Initialize( const SignalProperties&,
                                  const SignalProperties& Output )
{
  Halt();

  int numAChans;
  int numDChans = 0;
  bool useDigInputs = (int)Parameter("DigitalEnable");
  if (useDigInputs){
	numAChans = 8;
	numDChans = 1;
  }
  else{                 
	numAChans = Output.Channels();
  }

  string COMport = Parameter("COMport");
  mDev = GT_OpenDevice( const_cast<char*>( COMport.c_str() ) );
  if (mDev == NULL)
  {
    //there seems to be a bug (or feature...?) where if you open the device
    //after it is already opened, it actually disconnects the bluetooth connection
    //and it needs to be reconnected. This tests for this bug, since it will reconnect
    //on the 2nd try if possible; if there is another problem, it gives the error and returns
    mDev = GT_OpenDevice( const_cast<char*>( COMport.c_str() ) );
    if (mDev == NULL)
    {
      UINT lastErr;
      GT_GetLastError(&lastErr);
      _ERRSTR errorStr;
      GT_TranslateErrorCode(&errorStr,lastErr);
      bcierr << "Could not open device: " << errorStr.Error << endl;
      return;
    }
  }

  _AIN ain;
  _DIO dio;

  ain.ain1 = (numAChans > 0); // scan
  ain.ain2 = (numAChans > 1); // scan
  ain.ain3 = (numAChans > 2); // scan
  ain.ain4 = (numAChans > 3); // scan
  ain.ain5 = (numAChans > 4); // scan
  ain.ain6 = (numAChans > 5); // scan
  ain.ain7 = (numAChans > 6); // scan
  ain.ain8 = (numAChans > 7); // scan


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
	mDigState = 0;
	GT_SetDigitalOut(mDev, 0x10);
  }

  bool ret = GT_InitChannels(mDev, ain, dio); // init analog channels and digital lines on g.MOBIlab
  if (!ret)
  {
    bcierr << "Could not initialize device" << endl;
    return;
  }

  if (Parameter("TestMode") == 1)
	  GT_SetTestmode(mDev, true);
  else
  	GT_SetTestmode(mDev, false);
  ret = GT_StartAcquisition(mDev);
  if (!ret)
    bcierr << "Could not start data acquisition" << endl;

  int numSamplesPerScan = (numAChans+numDChans) * Output.Elements(),
      blockSize = numSamplesPerScan * sizeof( sint16 ),
      blockDuration = 1e3 * Output.Elements() / Parameter( "SamplingRate" );
  mpAcquisitionThread = new gMOBIlabThread( blockSize, blockDuration, mDev );
}


// **************************************************************************
// Function:   Process
// Purpose:    This function is called within the data acquisition loop
//             it fills its output signal with values
//             and does not return until all data has been acquired.
// Parameters: References to input signal (ignored) and output signal.
// Returns:    N/A
// **************************************************************************
void gMOBIlabPlusADC::Process( const GenericSignal&, GenericSignal& Output )
{

  const int cMaxAChans = 8;
  uint16 mask[] =
      {
        0x0001,
        0x0008, // intentionally out of sequence
        0x0002,
        0x0004,
        0x0010,
        0x0020,
        0x0040,
        0x0080
	  };
	  
  for( int sample = 0; sample < Output.Elements(); ++sample )
  {
    for( int channel = 0; channel < min( cMaxAChans, Output.Channels() ); ++channel )
      Output( channel, sample ) = mpAcquisitionThread->ExtractData();

    if( Output.Channels() > cMaxAChans )
    {
      //the digital lines are stored in a single sample, with the values in each bit
      // the order is (MSB) 8 7 6 5 2 4 3 1 (LSB)

      uint16 value = mpAcquisitionThread->ExtractData();
      for( int channel = cMaxAChans; channel < Output.Channels(); ++channel )
        Output( channel, sample ) = ( value & mask[ channel - cMaxAChans ] ) ? 100 : 0;
    }
  }

  if (mEnableDigOut){
  	mDigState = (mDigState+1) % 2;
	if (mDigState == 0)
		GT_SetDigitalOut(mDev, 0x10);
	else
		GT_SetDigitalOut(mDev, 0x11);
  }

  if( mpAcquisitionThread->IsTerminated() )
    bcierr << "Lost connection to device" << endl;
}


// **************************************************************************
// Function:   Halt
// Purpose:    This routine shuts down data acquisition.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void gMOBIlabPlusADC::Halt()
{
  delete mpAcquisitionThread;
  mpAcquisitionThread = NULL;

  if (mDev)
  {
    GT_StopAcquisition(mDev);
    GT_CloseDevice(mDev);
    mDev = NULL;
  }
}

