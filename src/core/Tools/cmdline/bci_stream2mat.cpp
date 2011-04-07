////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <cstdio>
#include <set>

#include "bci_tool.h"
#include "Param.h"
#include "ParamList.h"
#include "State.h"
#include "StateList.h"
#include "StateVector.h"
#include "GenericVisualization.h"
#include "MessageHandler.h"
#include "BCIError.h"
#include "Version.h"

using namespace std;

string ToolInfo[] =
{
  "bci_stream2mat",
  BCI2000_VERSION,
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
    miUTF16 = 17,

    mxCELL_CLASS   = 1,
    mxSTRUCT_CLASS = 2,
    mxCHAR_CLASS   = 4,
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
  StateList           mStatelist;
  StateVector*        mpStatevector;
  SignalProperties    mSignalProperties;
  typedef set<string> StringSet; // A set is a sorted container of unique values.
  StringSet           mStateNames;
  size_t              mDataElementSizePos,
                      mDataColsPos,
                      mDataSizePos,
                      mDataCols;

  size_t BeginVar(size_t flags) const;
  void FinishVar(size_t sizePos) const;
  void WriteDims(size_t nRows, size_t nCols) const;
  void WriteName(string name) const;
  
  void WriteHeader();
  void WriteData( const GenericSignal& );
  void Write16( unsigned short value ) const
  { mrOut.write( reinterpret_cast<const char*>( &value ), sizeof( value ) ); }
  void Write32( unsigned int value ) const
  { mrOut.write( reinterpret_cast<const char*>( &value ), sizeof( value ) ); }
  void WriteFloat32( float value ) const
  { mrOut.write( reinterpret_cast<const char*>( &value ), sizeof( value ) ); }
  void Pad() const;

  virtual bool HandleState(                 istream& );
  virtual bool HandleVisSignal(             istream& );
  virtual bool HandleVisSignalProperties(   istream& );
  virtual bool HandleStateVector(           istream& );
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


size_t
StreamToMat::BeginVar(size_t flags) const
{
  Write32( miMATRIX );
  size_t sizePos = mrOut.tellp();
  Write32( 0 ); // placeholder for size
  Write32( miUINT32 ); Write32( 8 ); // flags are coming
  Write32( flags ); Write32( 0 ); // here are the flags
  return sizePos;
}

void
StreamToMat::FinishVar(size_t sizePos) const
{ // bounce back to sizePos, write the number of bytes between here and where you just came from - 4, bounce back to where you came from
  size_t endPos = mrOut.tellp();
  mrOut.seekp( sizePos );
  Write32( endPos - sizePos - 4 );
  mrOut.seekp( endPos );
}

void
StreamToMat::WriteDims(size_t nRows, size_t nCols) const
{
  Write32( miINT32 ); Write32( 8 );
  Write32( nRows ); Write32( nCols );
}

void
StreamToMat::WriteName(string name) const
{
  Write32( miINT8 ); Write32( name.size() );
  if( name.size() )
  {
    mrOut << name;
    Pad();
  }
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
  size_t indexSizePos = BeginVar( mxSTRUCT_CLASS );
  WriteDims( 1, 1 );
  WriteName( "Index" );
  
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
    long sizePos = BeginVar( mxUINT32_CLASS );
    WriteDims( 1, 1 );
    WriteName( "" );
    // Array data
    Write32( 4 << 16 | miUINT32 ); Write32( i );

    FinishVar( sizePos );
  }
  // An array with the signal's dimensions holding the signal entries' row indices
  long numSignalEntries = mSignalProperties.Channels() * mSignalProperties.Elements();
  {
    long sizePos = BeginVar( mxUINT32_CLASS );
    WriteDims( mSignalProperties.Channels(), mSignalProperties.Elements() );
    WriteName( "" );
    // Array data
    Write32( miUINT32 ); Write32( 4 * numSignalEntries );
    for( int j = 0; j < mSignalProperties.Elements(); ++j )
      for( int i = 0; i < mSignalProperties.Channels(); ++i )
        Write32( mStateNames.size() + 1 + i * mSignalProperties.Elements() + j );
    FinishVar( sizePos );
    Pad();
  }
  FinishVar( indexSizePos );

  // A cell array of channel labels
  long channelLabelsSize = BeginVar( mxCELL_CLASS );
  WriteDims( mSignalProperties.Channels(), 1 );
  WriteName( "ChannelLabels" );
  for( int i = 0; i < mSignalProperties.Channels(); i++ )
  {
    string label = mSignalProperties.ChannelLabels()[i];
    size_t cellSizePos = BeginVar( mxCHAR_CLASS );
    WriteDims( 1, label.size() );
    WriteName( "" );
    Write32( miUTF16 ); Write32( 2 * label.size() ); for( int j = 0; j < label.size(); j++ ) Write16( label[j] );
    Pad();
    FinishVar( cellSizePos );
  }
  FinishVar( channelLabelsSize );

  // A cell array of element labels
  long elementLabelsSize = BeginVar( mxCELL_CLASS );
  WriteDims( mSignalProperties.Elements(), 1 );
  WriteName( "ElementLabels" );
  for( int i = 0; i < mSignalProperties.Elements(); i++ )
  {
    string str = mSignalProperties.ElementLabels()[i];
    size_t cellSizePos = BeginVar( mxCHAR_CLASS );
    WriteDims( 1, str.size() );
    WriteName( "" );
    Write32( miUTF16 ); Write32( 2 * str.size() ); for( int j = 0; j < str.size(); j++ ) Write16( str[j] );
    Pad();
    FinishVar( cellSizePos );
  }
  FinishVar( elementLabelsSize );

  // A single-precision numeric array of element values
  long elementValuesSize = BeginVar( mxSINGLE_CLASS );
  WriteDims( mSignalProperties.Elements(), 1 );
  WriteName( "ElementValues" );
  Write32( miSINGLE ); Write32( 4 * mSignalProperties.Elements() );
  for( int i = 0; i < mSignalProperties.Elements(); i++ )
  {
    float val = ( i - mSignalProperties.ElementUnit().Offset() ) * mSignalProperties.ElementUnit().Gain();
    WriteFloat32(val);
  }
  Pad();
  FinishVar( elementValuesSize );

  // A string denoting the element unit
  long elementUnitSize = BeginVar( mxCHAR_CLASS );
  string symbol = mSignalProperties.ElementUnit().Symbol();
  WriteDims( 1, symbol.size() );
  WriteName( "ElementUnit" );
  Write32( miUTF16 ); Write32( 2 * symbol.size() ); for( int j = 0; j < symbol.size(); j++ ) Write16( symbol[j] );
  Pad();
  FinishVar( elementUnitSize );
  
  // A single-precision numeric array that holds the signal.
  mDataElementSizePos = BeginVar( mxSINGLE_CLASS );
  WriteDims( mStateNames.size() + numSignalEntries, 0 );
  mDataColsPos = mrOut.tellp(); mDataColsPos -= 4;
  WriteName( "Data" );
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
      WriteFloat32( static_cast<float32>( mpStatevector->StateValue( i->c_str() ) ) );

  for( int i = 0; i < s.Channels(); ++i )
    for( int j = 0; j < s.Elements(); ++j )
      WriteFloat32( static_cast<float32>( s( i, j ) ) );
  ++mDataCols;
}

void
StreamToMat::FinishHeader() const
{
  FinishVar( mDataSizePos );
  Pad();
  FinishVar( mDataElementSizePos );
  
  size_t endPos = mrOut.tellp();
  mrOut.seekp( mDataColsPos );
  Write32( mDataCols );
  mrOut.seekp( endPos );
}

bool
StreamToMat::HandleState( istream& arIn )
{
  State s;
  s.ReadBinary( arIn );
  if( arIn )
  {
    mStatelist.Delete( s.Name() );
    mStatelist.Add( s );
    if( mpStatevector != NULL )
    {
      delete mpStatevector;
      mpStatevector = new StateVector( mStatelist );
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
  if( mSignalProperties.IsEmpty() )
    bcierr << "Internal error: HandleVisSignalProperties should have written the header already, but has not" << endl;
  if( s.Properties() != mSignalProperties )
    bcierr << "Ignored signal with inconsistent properties" << endl;
  else
    WriteData( s );
  return true;
}


bool
StreamToMat::HandleVisSignalProperties( istream& arIn )
{
  VisSignalProperties vsp;
  vsp.ReadBinary( arIn );
  mSignalProperties = vsp.SignalProperties();
  
  // Write a header first.
  mStateNames.clear();
  for( int i = 0; i < mStatelist.Size(); ++i )
    mStateNames.insert( mStatelist[ i ].Name() );
  WriteHeader();

  return true;
}


bool
StreamToMat::HandleStateVector( istream& arIn )
{
  if( mpStatevector == NULL )
    mpStatevector = new StateVector( mStatelist );
  mpStatevector->ReadBinary( arIn );
  return true;
}
