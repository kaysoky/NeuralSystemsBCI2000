/**
 * Program:   Biosemi2ADC
 * Module:    Biosemi2ADC.CPP

 * Version:   0.01
 * Copyright (C) 2005 Samuel A. Inverso (samuel.inverso@gmail.com), Yang Zhen

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 *
 * Revisions:
 *  Revision 1.1  2005/12/12 00:05:24  sinverso
 *  Initial Revision: Working and tested offline. Not tested in real experiments.
 *
 *  Revision 1.2  2005/12/14 15:24:15  mellinger
 *  Fixed state name typo in Process().
 *
 *  Revision 1.3 2008/10/25  Maria Laura  Blefari
 *  Fixed battery warning
 */

#include "PCHIncludes.h"
#pragma hdrstop

#include <vector>
#include <string>
#include <cassert>
#include "Biosemi2ADC.h"
#include "BCIError.h"
#include "GenericSignal.h"

#include "Biosemi2Client.h"

using namespace std;

// Register the source class with the framework.
RegisterFilter( Biosemi2ADC, 1 );

/*******************************************************************************
Function:   Biosemi2ADC
Purpose:    The constructor for the Biosemi2ADC
            it fills the provided list of parameters and states with the
            parameters and states it requests from the operator
Parameters: plist - the list of parameters
            slist - the list of states
Returns:    N/A
*******************************************************************************/
Biosemi2ADC::Biosemi2ADC()
: mSamplingRate( 0 ),
  mSoftwareCh(0),
  mSampleBlockSize(0),
  mPostfixTriggers(false),
  mTriggerScaleMultiplier(1)
{

 BEGIN_PARAMETER_DEFINITIONS
   "Source int SourceCh= 80 80 1 296 "
       "// number of digitized channels, includes triggers if postfix triggers is true.",
   "Source int SampleBlockSize= 120 5 1 % "
       "// number of samples per block",
   "Source int SamplingRate= 512 128 1 % "
       "// the signal sampling rate",
   "Source int PostfixTriggers= 1 0 0 1 "
        "// Make the triggers 16 channels and place them after end of EEG channels"
            " 0: no (warnning they will not be saved at all),"
            " 1: yes, post fix them (enumeration)",
   "Source int TriggerScaleMultiplier= 3000 1 1 % "
        "//number to multiply triggers by to scale them to the visualization range",
 END_PARAMETER_DEFINITIONS


 BEGIN_STATE_DEFINITIONS
   "Running 1 0 0 0",
   "BatteryLow 1 0 0 0",
   "MODE 4 0 0 0",      // 4 bits encode the speedmode. Bit 17,18,19 21 of the Status channel
   "MK2 1 0 0 0",
 END_STATE_DEFINITIONS

}

Biosemi2ADC::~Biosemi2ADC()
{
    Halt();
}


/*******************************************************************************
Function:   Preflight
Purpose:    Checks parameters for availability and consistence with input signal
            properties; requests minimally needed properties for the output
            signal; checks whether resources are available.
Parameters: Input and output signal properties pointers.
Returns:    N/A
*******************************************************************************/
void Biosemi2ADC::Preflight( const SignalProperties&,
                                   SignalProperties& outSignalProperties ) const
{

//Required states

    State("BatteryLow");
    State("MODE");
    State("MK2");


    if( Parameter("SampleBlockSize") < 1 ){
        bcierr << "Sample block Size of " << Parameter("SampleBlockSize")
               << " is less than 1" << endl;
    }

    if( Parameter("TriggerScaleMultiplier") <= 0 ){
        bcierr << "Trigger scale multiplier is: "
            << Parameter("TriggerScaleMultiplier")
            << ", but it must be greater than 0."
            << endl;
    }

    int reqChannels = Parameter("SourceCh");

// The number of Channels requested includes the triggers, so if the user wants
//to append the triggers, we need NUM_TRIGGERS less than software channels

    if( 1 == Parameter("PostfixTriggers" )){
        reqChannels -= Biosemi2Client::NUM_TRIGGERS;
        if( reqChannels <= 0 ){
            bcierr << "Requested eeg channels is <= 0. "
                << "You probably didn't mean this." << endl
                << "Remeber if you want to postfix triggers, softwareCh "
                << endl
                << "should equal the number of EEG channels you want + "
                << "the total number of Trigger channels ( "
                << Biosemi2Client::NUM_TRIGGERS << ")." << endl;

        }
    }
    else{
        bciout << "Warning: you are not post-fixing triggers."
                  " Triggers will not be saved." << endl;
        }

    mBiosemi.initialize(Parameter("SamplingRate"),
            Parameter("SampleBlockSize"), reqChannels, false );

    if( Parameter("SourceCh") > mBiosemi.getNumChannels() ){
        bcierr << "Number of channels requested, "
            << Parameter("SourceCh")
            << " is greater than the number"
            << endl
            << " of channels the biosemi can send, "
            << mBiosemi.getNumChannels()
            << ", at current mode: "
            << mBiosemi.getMode() << endl;
    }

    if( 0 != (mBiosemi.getSamplingRate() % (int)Parameter("SamplingRate")) ){
        bcierr << "Sampling rate requested: " << Parameter("SamplingRate")
            << " does not evenly divide biosemi sampling rate: "
            << mBiosemi.getSamplingRate() << endl;

    }

// Requested output signal properties.

   outSignalProperties = SignalProperties(
        Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ),
            SignalType::float32);
}


/*******************************************************************************
Function:   Initialize
Purpose:    This function parameterizes the Biosemi2ADC
            It is called each time the operator first starts,or suspends and then
            resumes, the system i.e., each time the system goes into the main
            data acquisition loop (fMain->MainDataAcqLoop())
Parameters: N/A
Returns:   N/A

*******************************************************************************/
void Biosemi2ADC::Initialize( const SignalProperties&, const SignalProperties& )
{
    mpDataBlock = &mBiosemi.getDataBlock();

// store the value of the needed parameters

    mSamplingRate = Parameter( "SamplingRate" );
    mSoftwareCh = Parameter("SourceCh");
    mSampleBlockSize = Parameter("SampleBlockSize");
    mTriggerScaleMultiplier = Parameter("TriggerScaleMultiplier");
    mPostfixTriggers = Parameter("PostfixTriggers") != 0;

    if( mPostfixTriggers ){
        mSignalChannels = mSoftwareCh - Biosemi2Client::NUM_TRIGGERS;
    }
    else{
        mSignalChannels = mSoftwareCh;
    }

    // Setup the State

    State("BatteryLow") = mBiosemi.isBatteryLow();
    if( State("BatteryLow" ) ){
        bciout << "Warning: Battery low " << endl;
    }
    State("MODE") =mBiosemi.getMode();
    State("MK2") = mBiosemi.isMK2();


}


/*******************************************************************************
Function:   Process
Purpose:    This function is called within fMain->MainDataAcqLoop()    it fills the
            already initialized array RawEEG with values and DOES NOT RETURN,
            UNTIL ALL DATA IS ACQUIRED
Parameters: N/A
Returns:
*******************************************************************************/
void Biosemi2ADC::Process( const GenericSignal&, GenericSignal& signal )
{

// wait for data to become ready

    mBiosemi.isDataReady();


// Make sure the block is valid.

    if( !mpDataBlock->isDataValid() ){
        if( mBiosemi.isBatteryLow() && !State("BatteryLow")){
            bciout << "Warning: Battery Low" << endl;

// we don't want to send messages to bicout everytime Process function is called,
// only send once and hope the user is paying attention.

            State("BatteryLow") = BATTERY_LOW;
        }
        else{
          // TODO  make this more descriptive
            bcierr << "Data is invalid for unkown reason." << endl;
        }
    }


// place the data and triggers to the out signal

    int triggerChan(0);
    for( int sample(0) ; sample < mSampleBlockSize ; ++sample ){

        for( int  channel(0); channel < mSoftwareCh ; ++channel ){
            triggerChan = 0;
            if( channel < mSignalChannels ){
                // this is a signal channel
                signal(channel, sample)=
                    mpDataBlock->getSignal(sample,channel)/8192.0;

//The USB receiver converts the 24-bit values to 32-bit integers by adding a
//least significant byte of zero's to every data word. So, in order to convert
//the incoming I32 values in ActiView to muV, the numbers should be divided by 8192

            }

            else if(mPostfixTriggers) {
                // this is a trigger channel
                signal(channel, sample)=
                    mpDataBlock->getTrigger(sample, triggerChan );
                ++triggerChan;
            }

        }
    }

}


/******************************************************************************
Function:   Halt
Purpose:    This routine shuts down data acquisition.
            In this special case, it does not do anything (since the random number
            generator does not have to be turned off)
Parameters: N/A
Returns:    N/A
*******************************************************************************/
void Biosemi2ADC::Halt()
{
    mBiosemi.halt();
}
