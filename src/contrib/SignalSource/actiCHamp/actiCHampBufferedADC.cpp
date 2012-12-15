////////////////////////////////////////////////////////////////////$
// Authors: Paul Ignatenko <paul dot ignatenko at gmail dot com
// Description: Source module for the actiCHamp
//
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with // this program.  If not, see <http://www.gnu.org/licenses/>.  //
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "actiCHampBufferedADC.h"
#include "BCIError.h"
#include "string"

#include "ThreadUtils.h" // for SleepFor()

using namespace std;


RegisterFilter( actiCHampBufferedADC, 1 );


actiCHampBufferedADC::actiCHampBufferedADC()
{

 BEGIN_PARAMETER_DEFINITIONS
	 "Source:ActiCHamp int actiCHampAmplifierID= 0 "
		"10 0 % // Amplifier device to use",

    "Source:ActiCHamp int Mode= 0 "
       "0 0 % // Set the numerical analog of the mode.",

	 "Source:Signal%20Properties float SamplingRate= 250 "
		"2000 0.0 % // Sampling rate at which to run the amplifier",

    "Source:Signal%20Properties int SampleBlockSize= 10 "
       "100 1 % // number of samples transmitted at a time",

   "Source:Signal%20Properties int SourceCh= 40 "
       "168 1 % // number of source channels"
	   
    "Source:Signal%20Properties floatlist SourceChGain= 40 "
    "1 1 1 1 1 1 1 1 1 1 "
    "1 1 1 1 1 1 1 1 1 1 "
    "1 1 1 1 1 1 1 1 1 1 "
    "1 1 1 1 1 1 1 1 1 1 "
    "% % % // channels gains",

    "Source:Signal%20Properties floatlist SourceChOffset= 40 "
    "0 0 0 0 0 0 0 0 0 0 "
    "0 0 0 0 0 0 0 0 0 0 "
    "0 0 0 0 0 0 0 0 0 0 "
    "0 0 0 0 0 0 0 0 0 0 "
    "% % % // channels offset",


    "Source:ActiCHamp int ReferenceChannel= -1 "
       "% % % // Set the library reference channel, use -1 for OFF",


 END_PARAMETER_DEFINITIONS


  // ...and likewise any state variables.

 /* BEGIN_STATE_DEFINITIONS */
 // TODO: BATTERY STATES

 /*  // IMPORTANT NOTE ABOUT BUFFEREDADC AND STATES: */  
 /*  // * BCI2000 States can only be populated "per sample" if they're populated synchronously. */
 /*  // * States can only be populated synchronously by calling State( "Y" )( idx ) = X; in OnProcess. */
 /*  // * States can also be populated asynchronously by calling bcievent << "Y" << x; in DoAcquire. */
 /*  // * Asynchronous state population using bcievent cannot occur per sample, but only per sampleblock. */
 /*  // * If a state changes multiple times in a single sample block, data WILL BE LOST using the bcievent interface. */
 /*  // * This presents a problem for many amplifier APIs because many of them must be instantiated and run in a single */
 /*  //   thread (they are not thread-safe) and access to API state data must be managed carefully if its needed in the main thread. */

 /*   "SomeState       8 0 0 0",    // These are just examples. Change them, or remove them. */
 /*   "SomeOtherState 16 0 0 0", */

 /* END_STATE_DEFINITIONS */

}

actiCHampBufferedADC::~actiCHampBufferedADC()
{
  myDevice.close();
}

void
actiCHampBufferedADC::OnHalt()
{
  myDevice.close();
}

void
actiCHampBufferedADC::OnPreflight( SignalProperties& Output ) const
{
  if( (int)Parameter("actiCHampAmplifierID") < 0)
  {
      bcierr << "Wrong amplifier ID " << endl;
  }
  if( (double)Parameter( "SamplingRate" ) == 0.0 )
    bcierr << "SamplingRate cannot be zero" << endl;

  if( 10000%(int)Parameter("SamplingRate") != 0)
      bcierr << "SamplingRate must go into 10000 evenly" << endl;

  if( (unsigned int)Parameter("ReferenceChannel") < 0 )
      bcierr << "ReferenceChannel must greater than 0"<<endl ;

  if( (unsigned int)Parameter("ReferenceChannel") > Parameter("SourceCh") )
      bcierr << "ReferenceChannel must be smaller than SourceCh"<<endl ;

  int samplingRate = Parameter("SamplingRate");
  int numberOfChannels = Parameter( "SourceCh" );
  int samplesPerBlock  = Parameter( "SampleBlockSize" );
  int referenceChannel = Parameter( "ReferenceChannel");
  SignalType sigType = SignalType::float32; 
  Output = SignalProperties( numberOfChannels, samplesPerBlock, sigType );

}

void
actiCHampBufferedADC::OnInitialize( const SignalProperties& Output )
{
  // The user has pressed "Set Config" and all Preflight checks have passed.
  // The system will now transition into an "Initialized" state.
  // The signal properties can no longer be modified, but the const limitation has gone, so
  // the actiCHampBufferedADC instance itself can be modified. Allocate any memory you need
  // store any information you need in private member variables.

  // As long as the system is in an initialized state, the module should be pushing data from
  // the amplifier through the system.  Initialized != Running, however.  The system can call
  // StartRun and StopRun many times while remaining initialized.  The system will only become
  // uninitialized again once OnHalt is called.

  // Don't bother with any amplifier-API stuff here, however: instead, do this in
  // OnStartAcquisition() which will be called in the acquisition thread immediately after this
  // method finishes.
  /* myDevice.open((int)Parameter("actiCHampAmplifierID")); */
  /* DeviceSettings settings; */
  /* settings.direct_config = true; */
  /* settings.rate = 250; */
  /* settings.mode = (t_champMode) 0; */
  /* settings.decimation = 40; */
  /* settings.averaging = 1; */

  /* LibrarySettings libset; */
  /* libset.debug = false; */
  /* libset.init_tries = 3; */

  /* myDevice.set_device_settings(settings); */
  /* myDevice.apply(); */


  /* double samplesPerSecond = Parameter( "SamplingRate" ); */
  /* mSampleBlockSize  = Parameter( "SampleBlockSize" ); */
  /* mMsecPerBlock = 1000.0 * mSampleBlockSize / samplesPerSecond; */



    referenceChannel = Parameter("referenceChannel");
    mode             = Parameter ("Mode");
    deviceNumber     = Parameter ("actiCHampAmplifierID");
    sampleRate       = Parameter( "SamplingRate" );
    mSampleBlockSize = Parameter( "SampleBlockSize" );
    mMsecPerBlock    = 1000.0 * mSampleBlockSize / sampleRate;
}

void 
actiCHampBufferedADC::OnStartAcquisition()
{
    myDevice.open(deviceNumber);
    myDevice.set_rate(sampleRate);
    myDevice.set_mode((t_champMode) mode);
    myDevice.set_reference_channel(referenceChannel);


    myDevice.init();

    if( !myDevice.start())
    {
      bcierr << "Device did not start";
    }

}

void actiCHampBufferedADC::DoAcquire( GenericSignal& Output ) 
{	
    myDevice.get_data(Output, mSampleBlockSize);
}

void
actiCHampBufferedADC::StartRun()
{
}

void
actiCHampBufferedADC::StopRun()
{
}

void
actiCHampBufferedADC::OnStopAcquisition()
{
  myDevice.stop();
}

