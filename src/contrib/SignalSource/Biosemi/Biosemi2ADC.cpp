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
 *
 *  Revison 2.0 2008/11/07  Jeremy Hill
 *  Updated design, including support for an auxiliary Analog Input Box (AIB)
 *  EEG + AIB acquisition tested---triggers not.
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
Parameters: N/A
Returns:    N/A
*******************************************************************************/
Biosemi2ADC::Biosemi2ADC()
: mSamplingRate( 0 ),
  mSourceCh(0),
  mSampleBlockSize(0),
  mChInd(NULL)
{

 BEGIN_PARAMETER_DEFINITIONS
   "Source int SourceCh= 80 80 1 296 "
       "// number of digitized channels, including AIB and trigger channels.",
   "Source int SampleBlockSize= 120 5 1 % "
       "// number of samples per block",
   "Source int SamplingRate=    512 128 1 % "
       "// the signal sampling rate",
   "Source intlist EEGChList=     64   1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64   % 1 % "
       "// list of physical channel indices for the EEG channels in use",
   "Source intlist AIBChList=      0          % 1 32"
       "// list of Auxiliary Input Box channels to acquire after the EEG channels (included in the SourceCh total)",
   "Source intlist TriggerChList= 16   1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16   % 1 16"
       "// list of one-bit trigger channels to append to the end (included in the SourceCh total)",
    "Source:Signal%20Properties:DataIOFilter floatlist SourceChOffset=  80     0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0            0       0       0       0       0       0       0       0       0       0       0       0       0       0       0       0        0 % %"
       "// offset for channels in A/D units",
    "Source:Signal%20Properties:DataIOFilter floatlist SourceChGain=    80     0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125 0.03125    100     100     100     100     100     100     100     100     100     100     100     100     100     100     100     100        % % %"
       "// gain for each channel (A/D units -> muV)",
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
Parameters: Input and output signal properties.
Returns:    N/A
*******************************************************************************/
void Biosemi2ADC::Preflight( const SignalProperties&,
                                   SignalProperties& Output ) const
{

//Required states

    State("BatteryLow");
    State("MODE");
    State("MK2");


    if( Parameter("SampleBlockSize") < 1 ){
        bcierr << "Sample block Size of " << Parameter("SampleBlockSize")
               << " is less than 1" << endl;
    }

    int reqChannels     = Parameter("SourceCh");
    int nEegRequested   = Parameter( "EEGChList" )->NumValues();
    int nAibRequested   = Parameter( "AIBChList" )->NumValues();
    int nTrigRequested  = Parameter( "TriggerChList" )->NumValues();
    int nTotalRequested = nEegRequested + nAibRequested + nTrigRequested;

    if ( reqChannels !=  nTotalRequested ) {
        bcierr << "Combined number of indices in EEGChList + AIBChList + TriggerChList"
               << " (" << nEegRequested
               << "+"  << nAibRequested
               << "+"  << nTrigRequested
               << "="  << nTotalRequested << ")"
               << " does not match SourceCh parameter value"
               << " (=" << reqChannels << ")"
               << endl;
    }

    mBiosemi.initialize(Parameter("SamplingRate"),
            Parameter("SampleBlockSize"), reqChannels);

    int eegChannelsAvailable = mBiosemi.getNumEEGChannels();
    for( int i = 0 ; i < nEegRequested ; ++i ) {
        int ind = Parameter( "EEGChList" )( i );
        if( ind < 1 || ind > eegChannelsAvailable ) {
            bcierr << "Illegal EEGChList index " << ind
                   << ". Legal range is [1," << eegChannelsAvailable << "]."
                   << endl;
        }
        double gain = Parameter( "SourceChGain" )( i );
        if( ::fabs( gain - 0.03125 ) > 1e-10) {
            bcierr << "SourceChGain should be equal to 0.03125 microvolts for all EEG channels";
            if( nEegRequested < reqChannels) bcierr << "(i.e. the first " << nEegRequested << " entries)";
            bcierr << " but the value for channel #" << (i+1) << " is " << gain << endl;
        }
    }

    int aibChannelsAvailable = mBiosemi.getNumAIBChannels();
    if( aibChannelsAvailable == 0 && nAibRequested != 0 ) {
        bcierr << "AIBChList must be empty if AIB box is not connected" << endl;
    }
    for( int i = 0 ; i < nAibRequested ; ++i ) {
        int ind = Parameter( "AIBChList" )( i );
        if( ind < 1 || ind > Biosemi2Client::NUM_AIB_CHANNELS ) {
            bcierr << "Illegal AIBChList index " << ind
                   << ". Legal range is [1," << Biosemi2Client::NUM_AIB_CHANNELS << "]."
                   << endl;
        } 
    }

    for( int i = 0 ; i < nTrigRequested ; ++i ) {
        int ind = Parameter( "TriggerChList" )( i );
        if( ind < 1 || ind > Biosemi2Client::NUM_TRIGGERS ) {
            bcierr << "Illegal TriggerChList index " << ind
                   << ". Legal range is [1," << Biosemi2Client::NUM_TRIGGERS << "]."
                   << endl;
        } 
    }

    if( 0 != (mBiosemi.getSamplingRate() % (int)Parameter("SamplingRate")) ){
        bcierr << "Sampling rate requested: " << Parameter("SamplingRate")
            << " does not evenly divide biosemi sampling rate: "
            << mBiosemi.getSamplingRate() << endl;

    }

// Requested output signal properties.

   Output = SignalProperties(
        Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ),
            SignalType::float32);
}


/*******************************************************************************
Function:   Initialize
Purpose:    This function parameterizes the Biosemi2ADC
Parameters: Input and output signal properties
Returns:    N/A

*******************************************************************************/
void Biosemi2ADC::Initialize( const SignalProperties&, const SignalProperties& )
{
    mpDataBlock = &mBiosemi.getDataBlock();

// store the value of the needed parameters

    mSamplingRate = Parameter( "SamplingRate" );
    mSourceCh = Parameter("SourceCh");
    mSampleBlockSize = Parameter("SampleBlockSize");

    delete [] mChInd;
    mChInd = new int[mSourceCh];
    int chInd_ind = 0;
    
    int nEegRequested   = Parameter( "EEGChList" )->NumValues();
    for( int i = 0 ; i < nEegRequested ; ++i ) {
        int ind = Parameter( "EEGChList" )( i );
        mChInd[chInd_ind++] = Biosemi2Client::FIRST_EEG_CHANNEL + ind - 1;
    }

    int nAibRequested   = Parameter( "AIBChList" )->NumValues();
    for( int i = 0 ; i < nAibRequested ; ++i ) {
        int ind = Parameter( "AIBChList" )( i );
        mChInd[chInd_ind++] = Biosemi2Client::FIRST_AIB_CHANNEL + ind - 1;
    }

    int nTrigRequested  = Parameter( "TriggerChList" )->NumValues();
    for( int i = 0 ; i < nTrigRequested ; ++i ) {
        int ind = Parameter( "TriggerChList" )( i );
        mChInd[chInd_ind++] = -ind;
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
Purpose:    Blocking mode data acquisition
Parameters: Input (ignored) and output signal
Returns:    N/A
*******************************************************************************/
void Biosemi2ADC::Process( const GenericSignal&, GenericSignal& Output )
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
            bcierr << "Data is invalid for unknown reason." << endl;
        }
    }


// place the data and triggers to the out signal

    int ind;
    for( int sample(0) ; sample < mSampleBlockSize ; ++sample ){
        for(int channel(0) ; channel < mSourceCh ; ++channel ) {
            ind = mChInd[channel];
            if ( ind < 0) {
                Output( channel, sample ) = mpDataBlock->getTrigger( sample, -ind );
            }
            else {
                Output( channel, sample ) = mpDataBlock->getSignal( sample, ind ) / 256.0;
            }
        }
    }

}


/******************************************************************************
Function:   Halt
Purpose:    Halting of all asynchronous activity.
Parameters: N/A
Returns:    N/A
*******************************************************************************/
void Biosemi2ADC::Halt()
{
    mBiosemi.halt();
}
