//////////////////////////////////////////////////////////////////////////////
//
// File: BCIReader.cpp
//
// Author: Juergen Mellinger
//
// Date: May 29, 2002
//
// Description: An base class for converting a BCI file with purely virtual
//              output functions (e.g., for output into a file, or directly
//              into an application via automation interfaces).
//
//////////////////////////////////////////////////////////////////////////////
#ifdef __BORLANDC__
#include <vcl.h>
#define VCL
#pragma hdrstop
#endif // __BORLANDC__

#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <assert>

#include "BCIReader.h"
#include "UBCIError.h"
#include "UState.h"
#include "UParameter.h"
#include "UGenericSignal.h"

using namespace std;
typedef set< STATE* > StateSet;
typedef map< STATE*, long > StatePosMap;
#define CALIBRATION_SIGNAL  0
#define TTD_MAX_SHORT (32000)
#define BCI2000_MAX_SHORT (MAXSHORT)

void
TBCIReader::Open( const char* inName )
{
    // Open the file.
    inputStream.close();
    inputStream.clear();
    fileName = "";
    inputStream.open( inName );
    if( !inputStream.is_open() )
        bcierr << "Could not open " << inName << " for text input." << endl;
    else
      fileName = inName;
}

void
TBCIReader::Process(    const TStrList& inChannelNames,
                        const TStrSet&  inIgnoreStates,
                              bool      inScanOnly )
{
    statesInFile.clear();
    
    string s;
    unsigned long   numSourceChannels = 0,
                    numTransmittedChannels = 0;
    long            dataOffset = 0,
                    sizeofStateVector = 0;

    bool success =
      inputStream >> s && s == "HeaderLen=" &&
      inputStream >> dataOffset >> s && s == "SourceCh=" &&
      inputStream >> numSourceChannels >> s && s == "StatevectorLen=" &&
      inputStream >> sizeofStateVector &&
      getline( inputStream, s );
    if( !success )
    {
      bcierr << "File format error in file " << fileName << ":" << endl
             << "Illegal header" << endl;
      return;
    }

    success =
      getline( inputStream, s ) && s == "[ State Vector Definition ] " &&
      getline( inputStream, s );

    STATELIST states;
    while( inputStream && ( s.length() == 0 || s[ 0 ] != '[' ) )
    {
        states.AddState2List( s.c_str() );
        success = success && getline( inputStream, s );
    }
    for( int i = 0; i < states.GetNumStates(); ++i )
      statesInFile.insert( states.GetStatePtr( i )->GetName() );

    STATEVECTOR stateVector( &states, true ),
                lastStateVector( stateVector );
    success = success && ( stateVector.GetStateVectorLength() == sizeofStateVector );
    if( !success )
    {
      bcierr << "File format error in file " << fileName << ":" << endl
             << "Bad state vector definition" << endl;
      return;
    }

    success = ( s == "[ Parameter Definition ] " );

    // Build a set of states that will be exported as markers.
    StateSet statesToConsider;
    for( int idx = 0; states.GetStatePtr( idx ) != NULL; ++idx )
    {
        STATE   *state = states.GetStatePtr( idx );
        if( inIgnoreStates.end() == inIgnoreStates.find( state->GetName() ) )
            statesToConsider.insert( states.GetStatePtr( idx ) );
    }

    PARAMLIST params;
    while( inputStream && s.length() != 0 )
    {
        params.AddParameter2List( s.c_str(), s.length() );
        success = success && getline( inputStream, s );
    }
    success = success && inputStream;
    if( !success )
    {
      bcierr << "File format error in file " << fileName << ":" << endl
             << "Bad parameter definition format" << endl;
      return;
    }
    inputStream.close();

    PARAM*       paramPtr = params.GetParamPtr( "SamplingRate" );
    const char*  valPtr;
    success = ( paramPtr != NULL && ( valPtr = paramPtr->GetValue() ) != NULL );
    if( !success )
    {
      bcierr << "File format error in file " << fileName << ":" << endl
             << "Missing parameter 'SamplingRate'" << endl;
      return;
    }
    float samplingRate = atof( valPtr );

    paramPtr = params.GetParamPtr( "SampleBlockSize" );
    success = ( paramPtr != NULL && ( valPtr = paramPtr->GetValue() ) != NULL );
    if( !success )
    {
      bcierr << "File format error in file " << fileName << ":" << endl
             << "Missing parameter 'SampleBlockSize'" << endl;
      return;
    }
    int sampleBlockSize = atoi( valPtr );

    vector< size_t > transmittedChannels;
    vector< double > calibrationFactors;
    vector< double > calibrationOffsets;

    // Happily applying a number of differring BCI2000 standards ...
    PARAM* sourceChOffset = params.GetParamPtr( "SourceChOffset" ),
         * sourceChGain = params.GetParamPtr( "SourceChGain" ),
         * transmitChList = params.GetParamPtr( "TransmitChList" ),
         * maxAmplitude = params.GetParamPtr( "MaxAmplitude" ),
         * caChList = params.GetParamPtr( "CAChList" ),
         * caMaxAmplitude = params.GetParamPtr( "CAMaxAmplitude" );

    if( caChList )
      transmitChList = caChList;
    if( caMaxAmplitude )
      maxAmplitude = caMaxAmplitude;

    if( sourceChOffset && sourceChGain )
    {
      // BCI2000 Calibration filter Parameters available.
      numTransmittedChannels = sourceChOffset->GetNumValues();
      if( sourceChGain->GetNumValues() != numTransmittedChannels )
      {
        bcierr << "File format error in file " << fileName << ":" << endl
               << "Number of entries in 'SourceChOffset' "
               << "does not meet that in 'SourceChGain'" << endl;
        return;
      }
      transmittedChannels.resize( numTransmittedChannels );
      calibrationFactors.resize( numTransmittedChannels );
      calibrationOffsets.resize( numTransmittedChannels );
      for( size_t channel = 0; channel < numTransmittedChannels; ++channel )
      {
        transmittedChannels[ channel ] = channel;
        calibrationFactors[ channel ] = atof( sourceChGain->GetValue( channel ) );
        calibrationOffsets[ channel ] = atof( sourceChOffset->GetValue( channel ) );
      }
    }
    else if( transmitChList && maxAmplitude )
    {
      // TTD Calibration filter Parameters available.
      numTransmittedChannels = transmitChList->GetNumValues();
      if( maxAmplitude->GetNumValues() != numTransmittedChannels )
      {
        bcierr << "File consistency error in file " << fileName << ":" << endl
               << "Number of values in 'MaxAmplitude' does not meet "
               << "number of values in 'TransmitChList'." << endl;
        return;
      }
      transmittedChannels.resize( numTransmittedChannels );
      calibrationFactors.resize( numTransmittedChannels );
      calibrationOffsets.resize( numTransmittedChannels );
      for( size_t channel = 0; channel < numTransmittedChannels; ++channel )
      {
        transmittedChannels[ channel ] = atoi( transmitChList->GetValue( channel ) );
        calibrationFactors[ channel ] = atof( maxAmplitude->GetValue( channel ) ) / double( TTD_MAX_SHORT );
        calibrationOffsets[ channel ] = 0.0;
      }
      if( *max_element( transmittedChannels.begin(), transmittedChannels.end() ) >= numSourceChannels )
      {
        bcierr << "File consistency error in file " << fileName << ":" << endl
               << "'TransmitChList' contains invalid channel number(s)" << endl;
        return;
      }
    }
    else
    {
      bcierr << "File format error in file " << fileName << ":" << endl
             << "Missing parameter(s) -- either 'SourceChOffset' and 'SourceChGain' "
             << "or 'TransmitChList' and 'MaxAmplitude' must be present." << endl;
      return;
    }

    inputStream.open( fileName.c_str(), ios_base::in | ios_base::binary );
    if( !inputStream.is_open() )
    {
        bcierr << "Could not re-open " << fileName << " for binary input." << endl;
        return;
    }

    long    dataLen = inputStream.seekg( 0, ios_base::end ).tellg()
                    - inputStream.seekg( dataOffset, ios_base::beg ).tellg(),
            bytesPerBlock = ( 2 * numSourceChannels  + sizeofStateVector ) * sampleBlockSize,
            numBlocks = dataLen / bytesPerBlock,
            bytesTooMuch = dataLen % bytesPerBlock,
            numSamples = numBlocks * sampleBlockSize;

    if( bytesTooMuch )
    {
        bcierr << "File consistency error in file " << fileName << ":" << endl
               << "Total data size is not integer multiple of block size ("
               << bytesTooMuch << " bytes remaining)" << endl;
        return;
    }

    if( !inScanOnly )
    {
      GenericIntSignal sourceSignal( numSourceChannels, sampleBlockSize );
      GenericSignal transmittedSignal( numTransmittedChannels, sampleBlockSize );

      TStrList channelNames = inChannelNames;
      while( channelNames.size() < numTransmittedChannels )
          channelNames.push_back( "" );

      TOutputInfo outputInfo =
      {
          fileName.c_str(),
          numTransmittedChannels,
          &channelNames,
          sampleBlockSize,
          numSamples,
          samplingRate
      };

      InitOutput( outputInfo );
      if( __bcierr.flushes() )
        return;

      StatePosMap StateSampleBeginPos;
      BYTE    *stateVectorData = stateVector.GetStateVectorPtr();

      inputStream.seekg( dataOffset, ios_base::beg );

      for( long block = 0; block < numBlocks; ++block )
      {
          if( !inputStream )
              return;

          for( int sample = 0; sample < sampleBlockSize; ++sample )
          {
              long curSamplePos = block * sampleBlockSize + sample;

              for( unsigned long channel = 0; channel < numSourceChannels; ++channel )
              {
                  short value;
                  inputStream.read( ( char* )&value, sizeof( value ) );
                  sourceSignal( channel, sample ) = value;
              }
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

              inputStream.read( ( char* )stateVectorData, sizeofStateVector );

              for( StateSet::iterator i = statesToConsider.begin();
                      i != statesToConsider.end(); ++i )
              {
                  STATE*  state = *i;
                  int     byteLoc = state->GetByteLocation(),
                          bitLoc =  state->GetBitLocation(),
                          length =  state->GetLength();
                  short   lastValue = lastStateVector.GetStateValue(
                                                  byteLoc, bitLoc, length ),
                          curValue = stateVector.GetStateValue(
                                                  byteLoc, bitLoc, length );

                  if( lastValue != curValue )
                  {
                      // We don't want zero states to show up as markers.
                      if( lastValue != 0 )
                      {
                          OutputStateRange( *state, lastValue, StateSampleBeginPos[ state ], curSamplePos );
                          if( __bcierr.flushes() )
                              return;
                      }

                      if( curValue == 0 )
                          StateSampleBeginPos.erase( state );
                      else
                          StateSampleBeginPos[ state ] = curSamplePos;

                      OutputStateChange( *state, curValue, curSamplePos );
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
        STATE*  state = i->first;
        int     byteLoc = state->GetByteLocation(),
                bitLoc =  state->GetBitLocation(),
                length =  state->GetLength();
        short   value = stateVector.GetStateValue( byteLoc, bitLoc, length );
        OutputStateRange( *state, value, i->second, numBlocks * sampleBlockSize );
        if( __bcierr.flushes() )
            return;
      }

      ExitOutput();
    }
}

