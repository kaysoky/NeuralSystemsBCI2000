////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: Class that provides an interface to the data stored in a
//              BCI2000 data file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCI2000FileReader.h"
#include "defines.h"

#include <fstream>
#include <sstream>

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
  static uint8 buf[ sizeof( T ) ];
  uint8* b = buf + sizeof( T );
  for( size_t i = 0; i < sizeof( T ); ++i )
    *--b = *p++;
  return *reinterpret_cast<const T*>( b );
}


// **************************************************************************
// Function:   BCI2000FileReader
// Purpose:    The constructor for the BCI2000FileReader object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
BCI2000FileReader::BCI2000FileReader()
: mpStatevector( NULL ),
  mpBuffer( NULL ),
  mfpReadValueBinary( NULL ),
  mErrorState( NoError )
{
}

BCI2000FileReader::BCI2000FileReader( const char* inFileName )
: mpStatevector( NULL ),
  mpBuffer( NULL ),
  mfpReadValueBinary( NULL ),
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
  delete mpStatevector;
  delete[] mpBuffer;
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
  mFile.close();
  mFile.clear();
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
  mDataSize = 2;
  mfpReadValueBinary = ReadValue<sint16>;
  mNumSamples = 0;
  mSignalProperties = ::SignalProperties( 0, 0, SignalType::int16 );
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

  mFile.open( inFilename, ios::in | ios::binary );
  if( !mFile.is_open() )
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
    throw "BCI2000FileReader::Parameter: Parameter does not exist";

  return ParamRef( const_cast<Param*>( param ) );
}

const StateRef
BCI2000FileReader::State( const std::string& name ) const
{
  int location = 0,
      length = 0;
  if( StateVector() != NULL && States() != NULL && States()->Exists( name ) )
  {
    const class State& state = ( *States() )[ name ];
    location = state.Location();
    length = state.Length();
    if( length < 1 )
      throw "BCI2000FileReader::State: Requested state has zero length";
  }
  else
    throw "BCI2000FileReader::State: Requested state is not accessible";

  return StateRef( const_cast<class StateVector*>( StateVector() ), location, length, 0 );
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
BCI2000FileReader::RawValue( int inChannel, long inSample )
{
  return mfpReadValueBinary( BufferSample( inSample ) + mDataSize * inChannel );
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
BCI2000FileReader::CalibratedValue( int inChannel, long inSample )
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
BCI2000FileReader::ReadStateVector( long inSample )
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
  mErrorState = MalformedHeader;

  // read the first line and do consistency checks
  string line, element;
  if( !getline( mFile, line, '\n' ) )
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

  SignalType signalType = SignalType::int16;
  if( linestream >> element )
  {
    if( element != "DataFormat=" )
      return;
    if( !( linestream >> signalType ) )
      return;
  }
  mDataSize = signalType.Size();
  // When running on a big endian machine, we need to swap bytes.
  bool isBigEndian = ( *reinterpret_cast<const uint16*>( "\0\1" ) == 0x0001 );
  if( isBigEndian )
  {
    switch( signalType )
    {
      case SignalType::int16:
        mfpReadValueBinary = ReadValue_SwapBytes<sint16>;
        break;
      case SignalType::int32:
        mfpReadValueBinary = ReadValue_SwapBytes<sint32>;
        break;
      case SignalType::float32:
        mfpReadValueBinary = ReadValue_SwapBytes<float32>;
        break;
    }
  }
  else
  { // Little endian machine
    switch( signalType )
    {
      case SignalType::int16:
        mfpReadValueBinary = ReadValue<sint16>;
        break;
      case SignalType::int32:
        mfpReadValueBinary = ReadValue<sint32>;
        break;
      case SignalType::float32:
        mfpReadValueBinary = ReadValue<float32>;
        break;
    }
  }
  // now go through the header and read all parameters and states
  getline( mFile >> ws, line, '\n' );
  if( line.find( "[ State Vector Definition ]" ) != 0 )
    return;
  while( getline( mFile, line, '\n' )
         && line.find( "[ Parameter Definition ]" ) == line.npos )
    mStatelist.Add( line.c_str() );
  while( getline( mFile, line, '\n' )
         && line != "" && line != "\r" )
    mParamlist.Add( line );

  // build statevector using specified positions
  mpStatevector = new ( class StateVector )( mStatelist );
  if( !mParamlist.Exists( "SamplingRate" ) )
    return;
  mSamplingRate = ::atoi( mParamlist[ "SamplingRate" ].Value().c_str() );

  // Read information about signal dimensions.
  int sampleBlockSize = 1;
  if( mParamlist.Exists( "SampleBlockSize" ) )
    sampleBlockSize = ::atoi( mParamlist[ "SampleBlockSize" ].Value().c_str() );
  mSignalProperties = ::SignalProperties( mChannels, sampleBlockSize, signalType );

  const float defaultOffset = 0.0;
  mSourceOffsets.clear();
  if( mParamlist.Exists( "SourceChOffset" ) )
  {
    const Param& SourceChOffset = mParamlist[ "SourceChOffset" ];
    for( int i = 0; i < SourceChOffset.NumValues(); ++i )
      mSourceOffsets.push_back( ::atof( SourceChOffset.Value( i ).c_str() ) );
  }
  mSourceOffsets.resize( mChannels, defaultOffset );

  const float defaultGain = 0.033;
  mSourceGains.clear();
  if( mParamlist.Exists( "SourceChGain" ) )
  {
    const Param& SourceChGain = mParamlist[ "SourceChGain" ];
    for( int i = 0; i < SourceChGain.NumValues(); ++i )
      mSourceGains.push_back( ::atof( SourceChGain.Value( i ).c_str() ) );
  }
  mSourceGains.resize( mChannels, defaultGain );

  if( mFile )
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
  if( mFile.is_open() )
  {
    streampos curPos = mFile.tellg();
    mFile.seekg( 0, ios_base::end );
    long dataSize = static_cast<long>( mFile.tellg() ) - mHeaderLength;
    mFile.seekg( curPos, ios_base::beg );
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
BCI2000FileReader::BufferSample( long inSample )
{
  if( inSample >= NumSamples() )
    throw "BCI2000FileReader::BufferSample: Sample position exceeds file size";
  int numChannels = SignalProperties().Channels();
  long filepos = HeaderLength()
               + inSample * ( mDataSize * numChannels + StateVectorLength() );
  if( filepos < mBufferBegin || filepos + mDataSize * numChannels + StateVectorLength() >= mBufferEnd )
  {
    if( !mFile.seekg( filepos, ios_base::beg ) )
      throw "BCI2000FileReader::BufferSample: Could not seek to sample position";

    mBufferBegin = filepos;
    mBufferEnd = mBufferBegin;
    while( mFile && ( mBufferEnd - mBufferBegin < StateVectorLength() ) )
    {
      mFile.read( mpBuffer + mBufferEnd - mBufferBegin,
        mBufferSize - ( mBufferEnd - mBufferBegin ) );
      mBufferEnd += mFile.gcount();
    }
    mFile.clear();
  }
  return mpBuffer + filepos - mBufferBegin;
}


