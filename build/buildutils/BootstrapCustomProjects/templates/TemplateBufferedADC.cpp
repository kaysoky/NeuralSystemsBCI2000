////////////////////////////////////////////////////////////////////////////////
// $Id: $
// Authors:
// Description: ` implementation
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
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "`.h"
#include "BCIError.h"

#include "ThreadUtils.h" // for SleepFor()

using namespace std;


RegisterFilter( `, 1 );
     // Change the location as appropriate, to determine where your filter gets
     // sorted into the chain. By convention:
     //  - filter locations within SignalSource modules begin with "1."
     //  - filter locations within SignalProcessing modules begin with "2."
     //       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
     //  - filter locations within Application modules begin with "3."


`::`()
{

  // Declare any parameters that the filter needs....

 BEGIN_PARAMETER_DEFINITIONS

    "Source:Signal%20Properties int SourceCh= 16 "
       "16 1 % // number of digitized and stored channels",

    "Source:Signal%20Properties int SampleBlockSize= 32 "
       "32 1 % // number of samples transmitted at a time",

    "Source:Signal%20Properties float SamplingRate= 256Hz "
       "256Hz 0.0 % // sample rate",

 END_PARAMETER_DEFINITIONS


  // ...and likewise any state variables.

 BEGIN_STATE_DEFINITIONS

  // IMPORTANT NOTE ABOUT BUFFEREDADC AND STATES:  
  // * BCI2000 States can only be populated "per sample" if they're populated synchronously.
  // * States can only be populated synchronously by calling State( "Y" )( idx ) = X; in OnProcess.
  // * States can also be populated asynchronously by calling bcievent << "Y" << x; in DoAcquire.
  // * Asynchronous state population using bcievent cannot occur per sample, but only per sampleblock.
  // * If a state changes multiple times in a single sample block, data WILL BE LOST using the bcievent interface.
  // * This presents a problem for many amplifier APIs because many of them must be instantiated and run in a single
  //   thread (they are not thread-safe) and access to API state data must be managed carefully if its needed in the main thread.

   "SomeState       8 0 0 0",    // These are just examples. Change them, or remove them.
   "SomeOtherState 16 0 0 0",

 END_STATE_DEFINITIONS

}

`::~`()
{
  // You should think twice before deallocating memory here as opposed to OnHalt().
  // OnHalt() is automatically called by BufferedADC upon destruction of this object.
}

void
`::OnHalt()
{
  // De-allocate any memory reserved in OnInitialize, stop any threads, etc.
  // Good practice is to write the OnHalt() method such that it is safe to call it even *before*
  // OnInitialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
  // have already been deleted:  set them to NULL after deletion).
  
  // Note that OnStopAcquisition() will be called immediately before this, in the acquisition
  // thread. OnStopAcquisition() is the proper place to do any amplifier-API cleanup.
}

void
`::OnPreflight( SignalProperties& Output ) const
{
  // The user has pressed "Set Config" and we need to sanity-check everything.
  // For example, check that all necessary parameters and states are accessible:
  //
  // Parameter( "Milk" );
  // State( "Bananas" );
  //
  // Also check that the values of any parameters are sane:
  //
  if( (double)Parameter( "SamplingRate" ) == 0.0 )
    bcierr << "SamplingRate cannot be zero" << endl;
  //
  // Errors issued in this way, during the Preflight phase, still allow the user
  // to open the Config dialog box, fix bad parameters and re-try.  By contrast,
  // errors and C++ exceptions at any other stage (outside Preflight) will make
  // the system stop, such that BCI2000 will need to be relaunched entirely.

  int numberOfChannels = Parameter( "SourceCh" );
  int samplesPerBlock  = Parameter( "SampleBlockSize" );
  SignalType sigType = SignalType::float32;  // could also parameterize this
  Output = SignalProperties( numberOfChannels, samplesPerBlock, sigType );

  //
  // Note that the ` instance itself, and its members, are read-only during the
  // Preflight phase---note the "const" at the end of the OnPreflight prototype above.
  // Any methods called by OnPreflight must also be declared as "const" in the same way.
}

void
`::OnInitialize( const SignalProperties& Output )
{
  // The user has pressed "Set Config" and all Preflight checks have passed.
  // The system will now transition into an "Initialized" state.
  // The signal properties can no longer be modified, but the const limitation has gone, so
  // the ` instance itself can be modified. Allocate any memory you need
  // store any information you need in private member variables.

  // As long as the system is in an initialized state, the module should be pushing data from
  // the amplifier through the system.  Initialized != Running, however.  The system can call
  // StartRun and StopRun many times while remaining initialized.  The system will only become
  // uninitialized again once OnHalt is called.

  // Don't bother with any amplifier-API stuff here, however: instead, do this in
  // OnStartAcquisition() which will be called in the acquisition thread immediately after this
  // method finishes.

  double samplesPerSecond = Parameter( "SamplingRate" );
  double samplesPerBlock  = Parameter( "SampleBlockSize" );
  mMsecPerBlock = 1000.0 * samplesPerBlock / samplesPerSecond;

  mLastTime = PrecisionTime::Now();
}

void 
`::OnStartAcquisition()
{
  // This method is called from the acquisition thread once the system is initialized.
  // You should use this space to start up your amplifier using its API.  Any initialization
  // here is done in the acquisition thread, so non-thread-safe APIs should work if initialized here.
}

void
`::DoAcquire( GenericSignal& Output )
{
  // Now we're acquiring a single SampleBlock of data in the acquisition thread.
  // Keep in mind that even though we're writing this data from another thread, the main thread
  // will block until we've written the SampleBlock, so be sure this is done in a timely manner 
  // or the system will not be able to perform in real-time.

  // Internally, BufferedADC writes this data to a buffer, then locks a mutex and pushes the data
  // into the GenericSignal output in the main thread.  The size of this buffer (in units of time)
  // is parameterized by "SourceBufferSize" declared by BufferedADC.  Be careful not to overflow!

  // For now, we output flat lines:
  for( int ch = 0; ch < Output.Channels(); ch++ )
    for( int el = 0; el < Output.Elements(); el++ )
      Output( ch, el ) = 0.0f;

  // Here is a wait loop to ensure that we do not deliver the signal faster than real-time
  // (In your final implementation, you should remove this: the hardware will play this role then.)
  while( PrecisionTime::UnsignedDiff( PrecisionTime::Now(), mLastTime ) < mMsecPerBlock ) ThreadUtils::SleepFor(1);
  mLastTime = PrecisionTime::Now();
}

void
`::StartRun()
{
  // The user has just pressed "Start" (or "Resume")
  bciout << "Hello World!" << endl;

  mLastTime = PrecisionTime::Now();
}

void
`::StopRun()
{
  // The Running state has been set to 0, either because the user has pressed "Suspend",
  // because the run has reached its natural end.
  bciout << "Goodbye World." << endl;
}

void
`::OnStopAcquisition()
{
  // This method is called from the acquisition thread just before it exits.  Use this method
  // to shut down the amplifier API (undoing whatever was done in OnStartAcquisition).
  // Immediately after this returns, the system will go into an un-initialized state and
  // OnHalt will be called in the main thread: (there you can undo whatever was done in
  // OnInitialize). 

  // This method will always be called before OnHalt is called.
}

