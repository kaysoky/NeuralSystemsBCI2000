////////////////////////////////////////////////////////////////////
// $Id$
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
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
  "bci_stream2table",
  BCI2000_VERSION,
  "Convert a binary BCI2000 stream into a human readable tabular form.",
  "Reads a BCI2000 compliant binary stream from standard input, "
    "and writes it to standard output "
    "as a tab/newline separated table of values.",
  "text",
  "",
};

class StreamToTable : public MessageHandler
{
 public:
  StreamToTable( ostream& arOut )
  : mrOut( arOut ), mpStatevector( NULL ), mSignalProperties( 0, 0 ),
    mInitialized( false ), mWriteoutPending( false ) {}
  ~StreamToTable() { delete mpStatevector; }
  void Finish();

 private:
  ostream&            mrOut;
  StateList           mStatelist;
  StateVector*        mpStatevector;
  SignalProperties    mSignalProperties;
  typedef set<string> StringSet; // A set is a sorted container of unique values.
  StringSet           mStateNames;
  bool                mInitialized,
                      mWriteoutPending;

  virtual bool HandleParam( istream& );
  virtual bool HandleState( istream& );
  virtual bool HandleVisSignalProperties( istream& );
  virtual bool HandleVisSignal( istream& );
  virtual bool HandleStateVector( istream& );

  void WriteOut( const GenericSignal& );
};

ToolResult
ToolInit()
{
  return noError;
}

ToolResult
ToolMain( OptionSet& arOptions, istream& arIn, ostream& arOut )
{
  if( arOptions.size() > 0 )
    return illegalOption;
  StreamToTable converter( arOut );
  while( arIn && arIn.peek() != EOF )
    converter.HandleMessage( arIn );
  converter.Finish();
  if( !arIn )
    return illegalInput;
  return noError;
}

void
StreamToTable::Finish()
{
  if( mWriteoutPending )
    WriteOut( GenericSignal() );
}

bool
StreamToTable::HandleParam( istream& arIn )
{
  return Param().ReadBinary( arIn );
}

bool
StreamToTable::HandleState( istream& arIn )
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
StreamToTable::HandleVisSignalProperties( istream& arIn )
{
  VisSignalProperties v;
  v.ReadBinary( arIn );
  mSignalProperties = v.SignalProperties();
  return true;
}

bool
StreamToTable::HandleVisSignal( istream& arIn )
{
  VisSignal v;
  v.ReadBinary( arIn );
  WriteOut( v );
  return true;
}

bool
StreamToTable::HandleStateVector( istream& arIn )
{
  static GenericSignal nullSignal;
  if( mWriteoutPending )
    WriteOut( nullSignal );
  if( mpStatevector == NULL )
    mpStatevector = new StateVector( mStatelist );
  mpStatevector->ReadBinary( arIn );
  mWriteoutPending = true;
  return true;
}

void
StreamToTable::WriteOut( const GenericSignal& inSignal )
{
  // Print a header line before the first line of data.
  if( !mInitialized )
  {
    mSignalProperties = inSignal.Properties();
    mrOut << "#";
    mStateNames.clear();
    for( int i = 0; i < mStatelist.Size(); ++i )
      mStateNames.insert( mStatelist[ i ].Name() );
    for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
      mrOut << "\t" << *i;
    for( int i = 0; i < inSignal.Channels(); ++i )
      for( int j = 0; j < inSignal.Elements(); ++j )
        mrOut << "\tSignal(" << mSignalProperties.ChannelLabels()[i] << "," << mSignalProperties.ElementLabels()[i] << ")";
    mrOut << endl;
    mInitialized = true;
  }
  if( inSignal.Properties() != mSignalProperties )
    bcierr << "Ignored signal with inconsistent properties" << endl;
  else
  {
    if( mpStatevector != NULL )
      for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
        mrOut << "\t" << mpStatevector->StateValue( i->c_str() );
    else
      for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
        mrOut << "\t0";

    for( int i = 0; i < inSignal.Channels(); ++i )
      for( int j = 0; j < inSignal.Elements(); ++j )
        mrOut << "\t" << inSignal( i, j );
    mrOut << endl;
  }
  mWriteoutPending = false;
}
