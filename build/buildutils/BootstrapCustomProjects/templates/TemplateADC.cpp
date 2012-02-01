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

#include "OSThread.h" // for OSThread::Sleep()

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


  // ...and likewise any state variables:

 BEGIN_STATE_DEFINITIONS

   "SomeState       8 0 0 0",    // These are just examples. Change them, or remove them.
   "SomeOtherState 16 0 0 0",

 END_STATE_DEFINITIONS

}


`::~`()
{
  Halt();
}

void
`::Halt()
{
  // De-allocate any memory reserved in Initialize, stop any threads, etc.
  // Good practice is to write the Halt() method such that it is safe to call it even *before*
  // Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
  // have already been deleted:  set them to NULL after deletion).
}

void
`::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
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
  // Errors issued in this way, during Preflight, still allow the user to open
  // the Config dialog box, fix bad parameters and re-try.  By contrast, errors
  // and C++ exceptions at any other stage (outside Preflight) will make the
  // system stop, such that BCI2000 will need to be relaunched entirely.

  int numberOfChannels = Parameter( "SourceCh" );
  int samplesPerBlock  = Parameter( "SampleBlockSize" );
  SignalType sigType = SignalType::float32;  // could also parameterize this
  Output = SignalProperties( numberOfChannels, samplesPerBlock, sigType );

  //
  // Note that the ` instance itself, and its members, are read-only during
  // this phase, due to the "const" at the end of the Preflight prototype above.
  // Any methods called by Preflight must also be "const" in the same way.
}


void
`::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  // The user has pressed "Set Config" and all Preflight checks have passed.
  // The signal properties can no longer be modified, but the const limitation has gone, so
  // the ` instance itself can be modified. Allocate any memory you need, start any
  // threads, store any information you need in private member variables.

  double samplesPerSecond = Parameter( "SamplingRate" );
  double samplesPerBlock  = Parameter( "SampleBlockSize" );
  mMsecPerBlock = 1000.0 * samplesPerBlock / samplesPerSecond;

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
`::Process( const GenericSignal& Input, GenericSignal& Output )
{

  // Now we're acquiring a single SampleBlock of data.
  // Remember not to take too much CPU time here, or you will break the real-time constraint.
  // To avoid losing data, it is advisable to create a separate thread which reads from the
  // hardware and writes into a ring buffer, and read from the buffer during Process.

  // For now, we output flat lines:
  for( int ch = 0; ch < Output.Channels(); ch++ )
    for( int el = 0; el < Output.Elements(); el++ )
      Output( ch, el ) = 0.0f;


  // Here is a wait loop to ensure that we do not deliver the signal faster than real-time
  // (In your final implementation, you should remove this: the hardware will play this role then.)
  while( PrecisionTime::UnsignedDiff( PrecisionTime::Now(), mLastTime ) < mMsecPerBlock ) OSThread::Sleep(1);
  mLastTime = PrecisionTime::Now();
}

void
`::StopRun()
{
  // The Running state has been set to 0, either because the user has pressed "Suspend",
  // because the run has reached its natural end.
  bciout << "Goodbye World." << endl;
  // You know, you can delete methods if you're not using them.
  // Remove the corresponding declaration from `.h too, if so.
}

