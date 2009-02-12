//////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An base class for converting a BCI2000 file into different
//   formats. Output formats are represented as descendants implementing
//   BCIReader's purely virtual output functions.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIReader.h"
#include "BCI2000FileReader.h"
#include "BCIError.h"
#include "State.h"
#include "Param.h"
#include "GenericSignal.h"

#include <algorithm>

using namespace std;
typedef set<const State*> StateSet;
typedef map<const State*, long> StatePosMap;

void
BCIReader::Open( const char* inName )
{
  // Open the file.
  mFileName = "";
  mInputData.Open( inName );
  if( mInputData.IsOpen() )
    mFileName = inName;
  else
    bcierr << "Could not open " << inName << " for input." << endl;
}

void
BCIReader::Process( const StringList& inChannelNames,
                    const StringSet&  inIgnoreStates,
                    bool              inScanOnly )
{
  mStatesInFile.clear();

  unsigned int numChannels = mInputData.SignalProperties().Channels();
  float        samplingRate = mInputData.SamplingRate();

  const StateList& states = *mInputData.States();
  for( int i = 0; i < states.Size(); ++i )
    mStatesInFile.insert( states[ i ].Name() );
  // Build a set of states that will be exported as markers.
  StateSet statesToConsider;
  for( int idx = 0; idx < states.Size(); ++idx )
    if( inIgnoreStates.end() == inIgnoreStates.find( states[ idx ].Name() ) )
      statesToConsider.insert( &states[ idx ] );

  StringList stateNames;
  for( StateSet::iterator i = statesToConsider.begin(); i != statesToConsider.end(); ++i )
    stateNames.push_back( ( *i )->Name() );

  int sampleBlockSize = mInputData.SignalProperties().Elements();
  long numSamples = mInputData.NumSamples(),
       numBlocks = numSamples / sampleBlockSize,
       samplesTooMuch = numSamples % sampleBlockSize;

  if( samplesTooMuch )
  {
    bcierr << "File consistency error in file " << mFileName << ":" << endl
           << "Total data size is not integer multiple of block size ("
           << samplesTooMuch << " samples remaining)" << endl;
    return;
  }

  if( !inScanOnly )
  {
    GenericSignal signal( numChannels, sampleBlockSize );
    StringList channelNames;
    if( mInputData.Parameters()->Exists( "ChannelNames" ) )
    {
      ParamRef ChannelNames = mInputData.Parameter( "ChannelNames" );
      for( int i = 0; i < ChannelNames->NumValues(); ++i )
        channelNames.push_back( ChannelNames( i ) );
    }
    for( size_t i = channelNames.size(); i < inChannelNames.size(); ++i )
      channelNames.push_back( inChannelNames[ i ] );
    while( channelNames.size() < numChannels )
      channelNames.push_back( "" );

    OutputInfo outputInfo =
    {
      mFileName.c_str(),
      numChannels,
      &channelNames,
      sampleBlockSize,
      numSamples,
      samplingRate,
      &stateNames
    };

    InitOutput( outputInfo );
    if( bcierr__.Flushes() )
      return;

    const StateVector& stateVector = *mInputData.StateVector();
    StateVector        lastStateVector( stateVector );
    StatePosMap        StateSampleBeginPos;

    for( long block = 0; block < numBlocks; ++block )
    {
      for( int sample = 0; sample < sampleBlockSize; ++sample )
      {
        long curSamplePos = block * sampleBlockSize + sample;

        for( unsigned long channel = 0; channel < numChannels; ++channel )
          signal( channel, sample ) = mInputData.CalibratedValue( channel, curSamplePos );

        mInputData.ReadStateVector( curSamplePos );
        for( StateSet::iterator i = statesToConsider.begin();
                i != statesToConsider.end(); ++i )
        {
          const State* pState = *i;
          int          location = pState->Location(),
                       length = pState->Length();
          short        lastValue = lastStateVector.StateValue( location, length ),
                       curValue = stateVector.StateValue( location, length );

          OutputStateValue( *pState, curValue, curSamplePos );
          if( bcierr__.Flushes() )
            return;
          if( lastValue != curValue )
          {
            // We don't want zero states to show up as markers.
            if( lastValue != 0 )
            {
              OutputStateRange( *pState, lastValue, StateSampleBeginPos[ pState ], curSamplePos );
              if( bcierr__.Flushes() )
                  return;
            }

            if( curValue == 0 )
              StateSampleBeginPos.erase( pState );
            else
              StateSampleBeginPos[ pState ] = curSamplePos;

            OutputStateChange( *pState, curValue, curSamplePos );
            if( bcierr__.Flushes() )
              return;
          }
        }
        lastStateVector = stateVector;
      }
      OutputSignal( signal, block * sampleBlockSize );
      if( bcierr__.Flushes() )
        return;
    }

    // If there are open state ranges, close them.
    for( StatePosMap::iterator i = StateSampleBeginPos.begin();
            i != StateSampleBeginPos.end(); ++i )
    {
      const State*  pState = i->first;
      int     location = pState->Location(),
              length = pState->Length();
      short   value = stateVector.StateValue( location, length );
      OutputStateRange( *pState, value, i->second, numBlocks * sampleBlockSize );
      if( bcierr__.Flushes() )
          return;
    }
    ExitOutput();
  }
}

void
BCIReader::Idle() const
{
#ifdef __BORLANDC__
  static time_t lastIdle = 0;
  time_t now = ::time( NULL );
  if( now - lastIdle > 0 )
  {
    lastIdle = now;
    Application->ProcessMessages();
  }
#endif // __BORLANDC__
}

