////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: Class that provides an interface to the data stored in a
//              BCI2000 data file.
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

#include "BCI2000FileReader.h"
#include "BCIException.h"
#include "defines.h"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>

#if _MSC_VER
# define ftello64 _ftelli64
# define fseeko64 _fseeki64
#endif // _MSC_VER

#if defined( __APPLE__ ) || defined( __BORLANDC__ )
# define ftello64 ftello
# define fseeko64 fseeko
#endif // __APPLE__ || __BORLANDC__

using namespace std;

// **************************************************************************
// Function:   ReadValue<DataType>
// Purpose:    Reads a value from the given memory location.
// Parameters: Pointer into memory buffer.
// Returns:    Data value.
// **************************************************************************
template<typename T>
static
GenericSignal::ValueType
ReadValue( const char* p )
{
  return *reinterpret_cast<const T*>( p );
}

// **************************************************************************
// Function:   ReadValue_SwapBytes<DataType>
// Purpose:    Reads a value from the given memory location, and swaps bytes.
// Parameters: Pointer into memory buffer.
// Returns:    Data value.
// **************************************************************************
template<typename T>
static
GenericSignal::ValueType
ReadValue_SwapBytes( const char* p )
{
  static uint8_t buf[ sizeof( T ) ];
  uint8_t* b = buf + sizeof( T );
  for( size_t i = 0; i < sizeof( T ); ++i )
    *--b = *p++;
  return *reinterpret_cast<const T*>( reinterpret_cast<char*>( b ) );
}


// **************************************************************************
// Function:   BCI2000FileReader
// Purpose:    The constructor for the BCI2000FileReader object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
BCI2000FileReader::BCI2000FileReader()
: mpStatevector( NULL ),
  mpFile( NULL ),
  mpBuffer( NULL ),
  mErrorState( NoError )
{
}

BCI2000FileReader::BCI2000FileReader( const char* inFileName )
: mpStatevector( NULL ),
  mpFile( NULL ),
  mpBuffer( NULL ),
  mErrorState( NoError )
{
  Open( inFileName );
}

// **************************************************************************
// Function:   ~BCI2000FileReader
// Purpose:    The destructor for the BCI2000FileReader object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
BCI2000FileReader::~BCI2000FileReader()
{
  Reset();
}

// **************************************************************************
// Function:   Reset
// Purpose:    Resets file related data members to a defined state.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
BCI2000FileReader::Reset()
{
  mInitialized = false;

  mParamlist.Clear();
  mStatelist.Clear();
  delete mpStatevector;
  mpStatevector = NULL;

  mFilename = "";
  if( mpFile )
  {
    ::fclose( mpFile );
    mpFile = NULL;
  }
  delete[] mpBuffer;
  mpBuffer = NULL;
  mBufferSize = 0;
  mBufferBegin = 0;
  mBufferEnd = 0;

  mFileFormatVersion = "n/a";
  mChannels = 0;
  mHeaderLength = 0;
  mStatevectorLength = 0;
  mSamplingRate = 1;
  mSignalType = SignalType::int16;
  mDataSize = mSignalType.Size();
  mSourceOffsets.clear();
  mSourceGains.clear();
  mNumSamples = 0;
  mSignalProperties = ::SignalProperties( 0, 0, mSignalType );

  mErrorState = NoError;
}

// **************************************************************************
// Function:   Open
// Purpose:    This method initializes the object by reading in
//             the header and creating a list of all parameters and states
// Parameters: filename - name of the file of interest
//             buf_size - size of input buffer to use
// **************************************************************************
BCI2000FileReader&
BCI2000FileReader::Open( const char* inFilename, int inBufSize )
{
  Reset();

  if( inFilename != NULL )
    mpFile = ::fopen( inFilename, "rb" );
  if( mpFile == NULL )
  {
    mErrorState = FileOpenError;
  }
  if( ErrorState() == NoError )
  {
    mFilename = inFilename;
    ReadHeader();
    if( ErrorState() == NoError )
    {
      CalculateNumSamples();
      mBufferSize = inBufSize;
      mpBuffer = new char[ mBufferSize ];
      mBufferBegin = 0;
      mBufferEnd = 0;
      mInitialized = true;
    }
  }
  return *this;
}

const ParamRef
BCI2000FileReader::Parameter( const std::string& name ) const
{
  const Param* param = NULL;
  if( Parameters() != NULL && Parameters()->Exists( name ) )
    param = &( *Parameters() )[ name ];
  else
    throw bciexception( "Parameter " << name << " does not exist" );

  return ParamRef( const_cast<Param*>( param ) );
}

const StateRef
BCI2000FileReader::State( const std::string& name ) const
{
  const class State* pState = NULL;
  if( StateVector() != NULL && States() != NULL && States()->Exists( name ) )
  {
    pState = &( *States() )[ name ];
    if( pState->Length() < 1 )
      throw bciexception( "Requested state " << name << " has zero length" );
  }
  else
    throw bciexception( "Requested state " << name << " is not accessible" );

  return StateRef( const_cast<class State*>( pState ), const_cast<class StateVector*>( StateVector() ), 0 );
}

// **************************************************************************
// Function:   RawValue
// Purpose:    Returns the sample value for a given sample and channel number
//             that is, the sample in the current run
// Parameters: channel - channel number
//             sample - sample number
// Returns:    value requested
// **************************************************************************
GenericSignal::ValueType
BCI2000FileReader::RawValue( int inChannel, long long inSample )
{
  GenericSignal::ValueType value = 0;
  const char* address = BufferSample( inSample ) + mDataSize * inChannel;

  // When running on a big endian machine, we need to swap bytes.
  static const bool isBigEndian = ( *reinterpret_cast<const uint16_t*>( "\0\1" ) == 0x0001 );
  if( isBigEndian )
  {
    switch( mSignalType )
    {
      case SignalType::int16:
        value = ReadValue_SwapBytes<int16_t>( address );
        break;
      case SignalType::int32:
        value = ReadValue_SwapBytes<int32_t>( address );
        break;
      case SignalType::float32:
        value = ReadValue_SwapBytes<float32_t>( address );
        break;
      default:
        break;
    }
  }
  else
  { // Little endian machine
    switch( mSignalType )
    {
      case SignalType::int16:
        value = ReadValue<int16_t>( address );
        break;
      case SignalType::int32:
        value = ReadValue<int32_t>( address );
        break;
      case SignalType::float32:
        value = ReadValue<float32_t>( address );
        break;
      default:
        break;
    }
  }
  return value;
}

// **************************************************************************
// Function:   CalibratedValue
// Purpose:    Returns the sample value in the .dat file for a given sample
//             and channel number that is, the sample in the current run,
//             in units of 1e-6 V, i.e. honouring the calibration parameters
//             present in the file.
// Parameters: channel - channel number
//             sample - sample number
// Returns:    value requested
// **************************************************************************
GenericSignal::ValueType
BCI2000FileReader::CalibratedValue( int inChannel, long long inSample )
{
  return ( RawValue( inChannel, inSample ) - mSourceOffsets[ inChannel ] ) * mSourceGains[ inChannel ];
}

// **************************************************************************
// Function:   ReadStateVector
// Purpose:    reads the statevector for a given sample
//             the results are in a statevector pointed to by "statevector"
// Parameters: sample - sample number
// Returns:    N/A
// **************************************************************************
BCI2000FileReader&
BCI2000FileReader::ReadStateVector( long long inSample )
{
  ::memcpy( ( *mpStatevector )( 0 ).Data(),
            BufferSample( inSample ) + mDataSize * SignalProperties().Channels(),
            StateVectorLength() );
  return *this;
}

// **************************************************************************
// Function:   ReadHeader
// Purpose:    This method reads the header of a BCI2000 data file
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
BCI2000FileReader::ReadHeader()
{
  ifstream file( mFilename.c_str(), ios::in | ios::binary );

  mErrorState = MalformedHeader;

  // read the first line and do consistency checks
  string line, element;
  if( !getline( file, line, '\n' ) )
    return;
  istringstream linestream( line );
  linestream >> element;
  if( element == "BCI2000V=" )
    linestream >> mFileFormatVersion >> element;
  else
    mFileFormatVersion = "1.0";
  if( element != "HeaderLen=" )
    return;

  linestream >> mHeaderLength >> element >> mChannels;
  if( element != "SourceCh=" )
    return;

  linestream >> element >> mStatevectorLength;
  if( element != "StatevectorLen=" )
    return;

  mSignalType = SignalType::int16;
  if( linestream >> element )
  {
    if( element != "DataFormat=" )
      return;
    if( !( linestream >> mSignalType ) )
      return;
  }
  mDataSize = mSignalType.Size();

  // now go through the header and read all parameters and states
  getline( file >> ws, line, '\n' );
  if( line.find( "[ State Vector Definition ]" ) != 0 )
    return;
  while( getline( file, line, '\n' )
         && line.find( "[ Parameter Definition ]" ) == line.npos )
    mStatelist.Add( line.c_str() );
  while( getline( file, line, '\n' )
         && line != "" && line != "\r" )
    mParamlist.Add( line );

  // build statevector using specified positions
  mpStatevector = new ( class StateVector )( mStatelist );
  if( !mParamlist.Exists( "SamplingRate" ) )
    return;
  mSamplingRate = PhysicalUnit()
                 .SetGain( 1.0 )
                 .SetOffset( 0.0 )
                 .SetSymbol( "Hz" )
                 .PhysicalToRaw( mParamlist[ "SamplingRate" ].Value() );

  // Read information about signal dimensions.
  int sampleBlockSize = 1;
  if( mParamlist.Exists( "SampleBlockSize" ) )
    sampleBlockSize = static_cast<int>( PhysicalUnit()
                                       .SetGain( 1.0 )
                                       .SetOffset( 0.0 )
                                       .SetSymbol( "" )
                                       .PhysicalToRaw( mParamlist[ "SampleBlockSize" ].Value().c_str() )
                                      );
  mSignalProperties = ::SignalProperties( mChannels, sampleBlockSize, mSignalType );
  mSignalProperties.ElementUnit().SetGain( 1.0 / mSamplingRate ).SetOffset( 0.0 ).SetSymbol( "s" );

  const float defaultOffset = 0.0;
  mSourceOffsets.clear();
  if( mParamlist.Exists( "SourceChOffset" ) )
  {
    const Param& SourceChOffset = mParamlist[ "SourceChOffset" ];
    for( int i = 0; i < SourceChOffset.NumValues(); ++i )
      mSourceOffsets.push_back( ::atof( SourceChOffset.Value( i ).c_str() ) );
  }
  mSourceOffsets.resize( mChannels, defaultOffset );

  const double defaultGain = 0.033;
  mSourceGains.clear();
  if( mParamlist.Exists( "SourceChGain" ) )
  {
    const Param& SourceChGain = mParamlist[ "SourceChGain" ];
    for( int i = 0; i < SourceChGain.NumValues(); ++i )
      mSourceGains.push_back( ::atof( SourceChGain.Value( i ).c_str() ) );
  }
  mSourceGains.resize( mChannels, defaultGain );

  if( file )
    mErrorState = NoError;
}

// **************************************************************************
// Function:   CalculateNumSamples
// Purpose:    Calculates the number of samples in the file
//             Assumes that ReadHeader() has been called before
//             if there is an unforeseen problem, number of samples is set to 0
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
BCI2000FileReader::CalculateNumSamples()
{
  mNumSamples = 0;
  if( mpFile )
  {
    long long curPos = ::ftello64( mpFile );
    ::fseeko64( mpFile, 0, SEEK_END );
    long long dataSize = ::ftello64( mpFile );
    dataSize -= mHeaderLength;
    ::fseeko64( mpFile, curPos, SEEK_SET );
    mNumSamples = dataSize / ( mDataSize * mChannels + mStatevectorLength );
  }
}

// **************************************************************************
// Function:   BufferSample
// Purpose:    Moves the data buffer such that it contains the given sample.
// Parameters: Sample position in file
// Returns:    The sample's buffer position
// **************************************************************************
const char*
BCI2000FileReader::BufferSample( long long inSample )
{
  if( inSample >= NumSamples() )
    throw bciexception( "Sample position " << inSample << " exceeds file size of " << NumSamples() );
  int numChannels = SignalProperties().Channels();
  long long filepos = HeaderLength() + inSample * ( mDataSize * numChannels + StateVectorLength() );
  if( filepos < mBufferBegin || filepos + mDataSize * numChannels + StateVectorLength() >= mBufferEnd )
  {
    if( 0 != ::fseeko64( mpFile, filepos, SEEK_SET ) )
      throw bciexception( "Could not seek to sample position" );

    mBufferBegin = filepos;
    mBufferEnd = mBufferBegin;
    while( !::feof( mpFile ) && !::ferror( mpFile ) && ( mBufferEnd - mBufferBegin < StateVectorLength() ) )
    {
      size_t bytesRead = ::fread( mpBuffer + ( mBufferEnd - mBufferBegin ), 1,
                                  mBufferSize - static_cast<size_t>( mBufferEnd - mBufferBegin ), mpFile );
      mBufferEnd += bytesRead;
    }
    ::clearerr( mpFile );
  }
  return mpBuffer + ( filepos - mBufferBegin );
}


