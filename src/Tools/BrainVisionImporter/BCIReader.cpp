//////////////////////////////////////////////////////////////////////////////
// $Id$
// File: BCIReader.cpp
//
// Author: Juergen Mellinger
//
// Date: May 29, 2002
//
// Description: An base class for converting a BCI file with purely virtual
//              output functions (e.g., for output into a file, or directly
//              into an application via automation interfaces).
// $Log$
// Revision 1.7  2006/03/15 14:52:58  mellinger
// Compatibility with BCB 2006.
//
// Revision 1.6  2006/02/21 17:10:43  mellinger
// Less strict consistency check between number of signal channels and number of offset/gain entries.
//
// Revision 1.5  2006/01/12 20:37:14  mellinger
// Adaptation to latest revision of parameter and state related class interfaces.
//
//////////////////////////////////////////////////////////////////////////////
#ifdef __BORLANDC__
#include <vcl.h>
#define VCL
#pragma hdrstop
#endif // __BORLANDC__

#include "BCIReader.h"
#include "UBCI2000Data.h"
#include "UBCIError.h"
#include "UState.h"
#include "UParameter.h"
#include "UGenericSignal.h"

#include <algorithm>

using namespace std;
typedef set< const STATE* > StateSet;
typedef map< const STATE*, long > StatePosMap;
#define CALIBRATION_SIGNAL  0
#define TTD_MAX_SHORT (32000)
#define BCI2000_MAX_SHORT (MAXSHORT)

void
TBCIReader::Open( const char* inName )
{
    // Open the file.
    mFileName = "";
    switch( mInputData.Initialize( inName ) )
    {
      case BCI2000ERR_NOERR:
        mFileName = inName;
        break;

      case BCI2000ERR_FILENOTFOUND:
        bcierr << "Could not open " << inName << " for input." << endl;
        break;

      case BCI2000ERR_MALFORMEDHEADER:
        bcierr << "File format error in file " << inName << ":" << endl
               << "Illegal header" << endl;
        break;

      default:
        bcierr << "An unspecific error occurred when trying to open "
               << inName << " for input." << endl;
    }
}

void
TBCIReader::Process(    const TStrList& inChannelNames,
                        const TStrSet&  inIgnoreStates,
                              bool      inScanOnly )
{
    mStatesInFile.clear();

	unsigned int numSourceChannels = mInputData.GetNumChannels(),
				 numTransmittedChannels = 0;
    float        samplingRate = mInputData.GetSamplingRate();

    const STATELIST& states = *mInputData.GetStateListPtr();
    for( size_t i = 0; i < states.Size(); ++i )
      mStatesInFile.insert( states[ i ].GetName() );
    // Build a set of states that will be exported as markers.
    StateSet statesToConsider;
    for( size_t idx = 0; idx < states.Size(); ++idx )
      if( inIgnoreStates.end() == inIgnoreStates.find( states[ idx ].GetName() ) )
        statesToConsider.insert( &states[ idx ] );

    TStrList stateNames;
    for( StateSet::iterator i = statesToConsider.begin(); i != statesToConsider.end(); ++i )
      stateNames.push_back( ( *i )->GetName() );

    const PARAMLIST& params = *mInputData.GetParamListPtr();
    const PARAM*     pParam = params.GetParamPtr( "SampleBlockSize" );
    const char*      pVal = NULL;
    bool success = ( pParam != NULL && ( pVal = pParam->GetValue() ) != NULL );
    if( !success )
    {
      bcierr << "File format error in file " << mFileName << ":" << endl
             << "Missing parameter 'SampleBlockSize'" << endl;
      return;
    }
    int sampleBlockSize = atoi( pVal );

    vector< size_t > transmittedChannels;
    vector< double > calibrationFactors;
    vector< double > calibrationOffsets;

    // Happily applying a number of differring BCI2000 standards ...
    const PARAM* pSourceChOffset = params.GetParamPtr( "SourceChOffset" ),
               * pSourceChGain = params.GetParamPtr( "SourceChGain" ),
               * pTransmitChList = params.GetParamPtr( "TransmitChList" ),
               * pMaxAmplitude = params.GetParamPtr( "MaxAmplitude" ),
               * pCaChList = params.GetParamPtr( "CAChList" ),
               * pCaMaxAmplitude = params.GetParamPtr( "CAMaxAmplitude" );

    if( pCaChList )
      pTransmitChList = pCaChList;
    if( pCaMaxAmplitude )
      pMaxAmplitude = pCaMaxAmplitude;

    if( pSourceChOffset && pSourceChGain )
    {
      // BCI2000 Calibration filter Parameters available.
      numTransmittedChannels = std::min( pSourceChGain->GetNumValues(), numSourceChannels );
      if( numTransmittedChannels != std::min( pSourceChOffset->GetNumValues(), numSourceChannels ) )
      {
        bcierr << "File format error in file " << mFileName << ":" << endl
               << "Number of entries in 'SourceChOffset' "
               << "does not match that in 'SourceChGain'" << endl;
        return;
      }
      if( numTransmittedChannels < numSourceChannels )
      {
        bcierr << "File format error in file " << mFileName << ":" << endl
               << "'SourceCh' header entry exceeds the "
               << "number of entries in 'SourceChGain' and 'SourceChOffset'" << endl;
        return;
      }
      transmittedChannels.resize( numTransmittedChannels );
      calibrationFactors.resize( numTransmittedChannels );
      calibrationOffsets.resize( numTransmittedChannels );
      for( size_t channel = 0; channel < numTransmittedChannels; ++channel )
      {
        transmittedChannels[ channel ] = channel;
        calibrationFactors[ channel ] = atof( pSourceChGain->GetValue( channel ) );
        calibrationOffsets[ channel ] = atof( pSourceChOffset->GetValue( channel ) );
      }
    }
    else if( pTransmitChList && pMaxAmplitude )
    {
      // TTD Calibration filter Parameters available.
      numTransmittedChannels = pTransmitChList->GetNumValues();
      if( pMaxAmplitude->GetNumValues() != numTransmittedChannels )
      {
        bcierr << "File consistency error in file " << mFileName << ":" << endl
               << "Number of values in 'MaxAmplitude' does not meet "
               << "number of values in 'TransmitChList'." << endl;
        return;
      }
      transmittedChannels.resize( numTransmittedChannels );
      calibrationFactors.resize( numTransmittedChannels );
      calibrationOffsets.resize( numTransmittedChannels );
      for( size_t channel = 0; channel < numTransmittedChannels; ++channel )
      {
        transmittedChannels[ channel ] = atoi( pTransmitChList->GetValue( channel ) );
        calibrationFactors[ channel ] = atof( pMaxAmplitude->GetValue( channel ) ) / double( TTD_MAX_SHORT );
        calibrationOffsets[ channel ] = 0.0;
      }
      if( *max_element( transmittedChannels.begin(), transmittedChannels.end() ) >= numSourceChannels )
      {
        bcierr << "File consistency error in file " << mFileName << ":" << endl
               << "'TransmitChList' contains invalid channel number(s)" << endl;
        return;
      }
    }
    else
    {
      bcierr << "File format error in file " << mFileName << ":" << endl
             << "Missing parameter(s) -- either 'SourceChOffset' and 'SourceChGain' "
             << "or 'TransmitChList' and 'MaxAmplitude' must be present." << endl;
      return;
    }

    long numSamples = mInputData.GetNumSamples(),
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
      GenericSignal sourceSignal( numSourceChannels, sampleBlockSize ),
                    transmittedSignal( numTransmittedChannels, sampleBlockSize );

      TStrList channelNames = inChannelNames;
      while( channelNames.size() < numTransmittedChannels )
        channelNames.push_back( "" );

      TOutputInfo outputInfo =
      {
        mFileName.c_str(),
        numTransmittedChannels,
        &channelNames,
        sampleBlockSize,
        numSamples,
        samplingRate,
        &stateNames
      };

      InitOutput( outputInfo );
      if( __bcierr.flushes() )
        return;

      const STATEVECTOR& stateVector = *mInputData.GetStateVectorPtr();
      STATEVECTOR        lastStateVector( stateVector );
      StatePosMap        StateSampleBeginPos;
      
      for( long block = 0; block < numBlocks; ++block )
      {
        for( int sample = 0; sample < sampleBlockSize; ++sample )
        {
          long curSamplePos = block * sampleBlockSize + sample;

          for( unsigned long channel = 0; channel < numSourceChannels; ++channel )
            sourceSignal( channel, sample ) = mInputData.ReadValue( channel, curSamplePos );

          for( unsigned long channel = 0; channel < numTransmittedChannels; ++channel )
          {
#if defined( CALIBRATION_SIGNAL ) && CALIBRATION_SIGNAL
             transmittedSignal( channel, sample ) = ( 10.0 * sample ) / sampleBlockSize - 5.0;
#else
            float value = sourceSignal( transmittedChannels[ channel ], sample );
            value -= calibrationOffsets[ channel ];
            value *= calibrationFactors[ channel ];
            transmittedSignal( channel, sample ) = value;
#endif
          }

          mInputData.ReadStateVector( curSamplePos );
          for( StateSet::iterator i = statesToConsider.begin();
                  i != statesToConsider.end(); ++i )
          {
            const STATE* pState = *i;
            int          location = pState->GetLocation(),
                         length = pState->GetLength();
            short        lastValue = lastStateVector.GetStateValue(
                                                           location, length ),
                         curValue = stateVector.GetStateValue(
                                                           location, length );

            OutputStateValue( *pState, curValue, curSamplePos );
            if( __bcierr.flushes() )
              return;
            if( lastValue != curValue )
            {
              // We don't want zero states to show up as markers.
              if( lastValue != 0 )
              {
                OutputStateRange( *pState, lastValue, StateSampleBeginPos[ pState ], curSamplePos );
                if( __bcierr.flushes() )
                    return;
              }

              if( curValue == 0 )
                StateSampleBeginPos.erase( pState );
              else
                StateSampleBeginPos[ pState ] = curSamplePos;

              OutputStateChange( *pState, curValue, curSamplePos );
              if( __bcierr.flushes() )
                return;
            }
          }
          lastStateVector = stateVector;
        }
        OutputSignal( transmittedSignal, block * sampleBlockSize );
        if( __bcierr.flushes() )
          return;
      }

      // If there are open state ranges, close them.
      for( StatePosMap::iterator i = StateSampleBeginPos.begin();
              i != StateSampleBeginPos.end(); ++i )
      {
        const STATE*  pState = i->first;
        int     location = pState->GetLocation(),
                length = pState->GetLength();
        short   value = stateVector.GetStateValue( location, length );
        OutputStateRange( *pState, value, i->second, numBlocks * sampleBlockSize );
        if( __bcierr.flushes() )
            return;
      }
      ExitOutput();
    }
}

void
TBCIReader::Idle() const
{
#ifdef VCL
    static time_t lastIdle = 0;
    time_t now = ::time( NULL );
    if( now - lastIdle > 0 )
    {
      lastIdle = now;
      Application->ProcessMessages();
    }
#endif
}
