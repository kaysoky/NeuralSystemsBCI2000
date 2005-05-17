////////////////////////////////////////////////////////////////////
// File:    bci_stream2mat.cpp
// Date:    Feb 22, 2005
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <set>

#include "bci_tool.h"
#include "shared/UParameter.h"
#include "shared/UState.h"
#include "shared/UGenericVisualization.h"
#include "shared/MessageHandler.h"

using namespace std;

string ToolInfo[] =
{
  "bci_stream2mat",
  "version 0.1.0, compiled "__DATE__,
  "Convert a binary BCI2000 stream into a matlab .mat file",
  "Reads a BCI2000 compliant binary stream from standard input, "
    "and writes it to stdout in matlab level 5 MAT-file format.",
  ""
};

class StreamToMat : public MessageHandler
{
  // Matlab file format related constants.
  enum
  {
    matTextHeaderLength = 116,
    matVersionInfoOffset = 124,
    matPadding = 8, // padding to 64 bit boundaries

    miMATRIX = 14,

    miINT8 = 1,
    miINT32 = 5,
    miUINT8 = 2,
    miUINT32 = 6,
    miSINGLE = 7,

    mxSTRUCT_CLASS = 2,
    mxDOUBLE_CLASS = 6,
    mxSINGLE_CLASS = 7,
    mxUINT32_CLASS = 13,
  };

 public:
  StreamToMat( ostream& arOut )
  : mrOut( arOut ), mpStatevector( NULL ), mSignalProperties( 0, 0 ),
    mDataElementSizePos( 0 ), mDataColsPos( 0 ), mDataSizePos( 0 ), mDataCols( 0 ) {}
  ~StreamToMat() { delete mpStatevector; }
  void FinishHeader() const;

 private:
  ostream&            mrOut;
  STATELIST           mStatelist;
  STATEVECTOR*        mpStatevector;
  SignalProperties    mSignalProperties;
  typedef set<string> StringSet; // A set is a sorted container of unique values.
  StringSet           mStateNames;
  size_t              mDataElementSizePos,
                      mDataColsPos,
                      mDataSizePos,
                      mDataCols;

  void WriteHeader();
  void WriteData( const GenericSignal& );
  void Write16( unsigned short value ) const
  { mrOut.write( reinterpret_cast<const char*>( &value ), sizeof( value ) ); }
  void Write32( unsigned int value ) const
  { mrOut.write( reinterpret_cast<const char*>( &value ), sizeof( value ) ); }
  void WriteFloat32( float value ) const
  { mrOut.write( reinterpret_cast<const char*>( &value ), sizeof( value ) ); }
  void Pad() const;

  virtual bool HandleSTATE(       istream& );
  virtual bool HandleVisSignal(   istream& );
  virtual bool HandleSTATEVECTOR( istream& );
};

ToolResult
ToolInit()
{
  return noError;
}

ToolResult
ToolMain( const OptionSet& arOptions, istream& arIn, ostream& arOut )
{
  if( arOptions.size() > 1 )
    return illegalOption;
  StreamToMat converter( arOut );
  while( arIn && arIn.peek() != EOF )
    converter.HandleMessage( arIn );
  converter.FinishHeader();
  if( !arIn )
    return illegalInput;
  return noError;
}

void
StreamToMat::Pad() const
{
  for( int i = mrOut.tellp(); i % matPadding; ++i )
    mrOut.put( 0 );
}

void
StreamToMat::WriteHeader()
{
  time_t timer = ::time( NULL );
  mrOut << "MATLAB 5.0 MAT-file created "
        << ::ctime( &timer )
        << " by "
        << ToolInfo[ name ] << ", "
        << ToolInfo[ version ];
  for( int i = mrOut.tellp(); i < matTextHeaderLength; ++i )
    mrOut << ' ';
  mrOut.seekp( matTextHeaderLength );
  for( int i = mrOut.tellp(); i < matVersionInfoOffset; ++i )
    mrOut << ' ';
  Write16( 0x0100 ); Write16( 'MI' );
  // Write a matlab structure containing arrays with state names
  // pointing to the associated columns.
  Write32( miMATRIX );
  size_t indexSizePos = mrOut.tellp();
  Write32( 0 );
  // Array flags
  Write32( miUINT32 ); Write32( 8 );
  Write32( mxSTRUCT_CLASS ); Write32( 0 );
  // Dimensions array
  Write32( miINT32 ); Write32( 8 );
  Write32( 1 ); Write32( 1 );
  // Array name
  const char indexName[] = "Index";
  Write32( miINT8 ); Write32( sizeof( indexName ) - 1 );
  mrOut << indexName;
  Pad();
  // Field name length
  const char signalName[] = "Signal";
  size_t fieldNameLength = sizeof( signalName ) - 1;
  for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
    if( i->length() > fieldNameLength )
      fieldNameLength = i->length();
  fieldNameLength = ( fieldNameLength / matPadding + 1 ) * matPadding;
  Write32( 4 << 16 | miINT32 ); Write32( fieldNameLength );
  // Field names
  Write32( miINT8 ); Write32( fieldNameLength * ( mStateNames.size() + 1 ) );
  for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
  {
    mrOut << *i;
    for( size_t j = i->length(); j < fieldNameLength; ++j )
      mrOut.put( '\0' );
  }
  mrOut << signalName;
  for( size_t j = sizeof( signalName ) - 1; j < fieldNameLength; ++j )
    mrOut.put( '\0' );
  // Fields
  // 1x1 arrays holding the states' column indices
  for( size_t i = 1; i <= mStateNames.size(); ++i )
  {
    Write32( miMATRIX );
    long sizePos = mrOut.tellp();
    Write32( 0 );
    // Array flags
    Write32( miUINT32 ); Write32( 8 );
    Write32( mxUINT32_CLASS ); Write32( 0 );
    // Dimensions array
    Write32( miINT32 ); Write32( 8 );
    Write32( 1 ); Write32( 1 );
    // Array name
    Write32( miINT8 ); Write32( 0 );
    // Array data
    Write32( 4 << 16 | miUINT32 ); Write32( i );

    long endPos = mrOut.tellp();
    mrOut.seekp( sizePos );
    Write32( endPos - sizePos - 4 );
    mrOut.seekp( endPos );
  }
  // An array with the signal's dimensions holding the signal entries' row indices
  long numSignalEntries = mSignalProperties.Channels() * mSignalProperties.Elements();
  {
    Write32( miMATRIX );
    long sizePos = mrOut.tellp();
    Write32( 0 );
    // Array flags
    Write32( miUINT32 ); Write32( 8 );
    Write32( mxUINT32_CLASS ); Write32( 0 );
    // Dimensions array
    Write32( miINT32 ); Write32( 8 );
    Write32( mSignalProperties.Channels() ); Write32( mSignalProperties.Elements() );
    // Array name
    Write32( miINT8 ); Write32( 0 );
    // Array data
    Write32( miUINT32 ); Write32( 4 * numSignalEntries );
    for( size_t j = 0; j < mSignalProperties.Elements(); ++j )
      for( size_t i = 0; i < mSignalProperties.Channels(); ++i )
        Write32( mStateNames.size() + 1 + i * mSignalProperties.Elements() + j );
    long endPos = mrOut.tellp();
    mrOut.seekp( sizePos );
    Write32( endPos - sizePos - 4 );
    mrOut.seekp( endPos );
    Pad();
  }
  long endPos = mrOut.tellp();
  mrOut.seekp( indexSizePos );
  Write32( endPos - indexSizePos - 4 );
  mrOut.seekp( endPos );

  // An array that holds the signal.
  Write32( miMATRIX );
  mDataElementSizePos = mrOut.tellp();
  Write32( 0 );
  // Array flags
  Write32( miUINT32 ); Write32( 8 );
  Write32( mxSINGLE_CLASS ); Write32( 0 );
  // Dimensions array
  Write32( miINT32 ); Write32( 8 );
  Write32( mStateNames.size() + numSignalEntries );
  mDataColsPos = mrOut.tellp();
  Write32( 0 );
  // Array name
  const char dataName[] = "Data";
  Write32( miINT8 ); Write32( sizeof( dataName ) - 1 );
  mrOut << dataName;
  Pad();
  Write32( miSINGLE );
  mDataSizePos = mrOut.tellp();
  Write32( 0 );
}

void
StreamToMat::WriteData( const GenericSignal& s )
{
  if( mpStatevector == NULL )
    for( size_t i = 0; i < mStateNames.size(); ++i )
      WriteFloat32( 0 );
  else
    for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
      WriteFloat32( mpStatevector->GetStateValue( i->c_str() ) );

  for( size_t i = 0; i < s.Channels(); ++i )
    for( size_t j = 0; j < s.Elements(); ++j )
      WriteFloat32( s( i, j ) );
  ++mDataCols;
}

void
StreamToMat::FinishHeader() const
{
  size_t endPos = mrOut.tellp();
  mrOut.seekp( mDataSizePos );
  Write32( endPos - mDataSizePos - 4 );
  mrOut.seekp( endPos );

  Pad();
  endPos = mrOut.tellp();
  mrOut.seekp( mDataElementSizePos );
  Write32( endPos - mDataElementSizePos - 4 );
  mrOut.seekp( endPos );

  mrOut.seekp( mDataColsPos );
  Write32( mDataCols );
  mrOut.seekp( endPos );
}

bool
StreamToMat::HandleSTATE( istream& arIn )
{
  STATE s;
  s.ReadBinary( arIn );
  if( arIn )
  {
    mStatelist.AddState2List( &s );
    if( mpStatevector != NULL )
    {
      delete mpStatevector;
      mpStatevector = new STATEVECTOR( &mStatelist, true );
    }
  }
  return true;
}

bool
StreamToMat::HandleVisSignal( istream& arIn )
{
  VisSignal v;
  v.ReadBinary( arIn );
  const GenericSignal& s = v;
  // Print a header line before the first line of data.
  if( mSignalProperties.IsEmpty() )
  {
    mSignalProperties = s.GetProperties();
    mStateNames.clear();
    for( int i = 0; i < mStatelist.GetNumStates(); ++i )
      mStateNames.insert( mStatelist.GetStatePtr( i )->GetName() );
    WriteHeader();
  }
  if( s.GetProperties() != mSignalProperties )
    bcierr << "Ignored signal with inconsistent properties" << endl;
  else
    WriteData( s );
  return true;
}

bool
StreamToMat::HandleSTATEVECTOR( istream& arIn )
{
  if( mpStatevector == NULL )
    mpStatevector = new STATEVECTOR( &mStatelist, true );
  mpStatevector->ReadBinary( arIn );
  return true;
}
