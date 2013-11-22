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
#include "MessageChannel.h"
#include "BCIError.h"
#include "Version.h"

using namespace std;

string ToolInfo[] =
{
  "bci_stream2table",
  PROJECT_VERSION,
  "Convert a binary BCI2000 stream into a human readable tabular form.",
  "Reads a BCI2000 compliant binary stream from standard input, "
    "and writes it to standard output "
    "as a tab/newline separated table of values.",
  "text",
  "",
};

class StreamToTable : public MessageChannel
{
 public:
  StreamToTable( istream& is, ostream& os )
  : MessageChannel( is, os ), mpStatevector( NULL ), mSignalProperties( 0, 0 ),
    mInitialized( false ), mWriteoutPending( false ) {}
  ~StreamToTable() { delete mpStatevector; }
  void Finish();

 private:
  StateList           mStatelist;
  StateVector*        mpStatevector;
  SignalProperties    mSignalProperties;
  typedef set<string> StringSet; // A set is a sorted container of unique values.
  StringSet           mStateNames;
  bool                mInitialized,
                      mWriteoutPending;

  virtual bool OnParam( istream& );
  virtual bool OnState( istream& );
  virtual bool OnVisSignalProperties( istream& );
  virtual bool OnVisSignal( istream& );
  virtual bool OnStateVector( istream& );

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
  StreamToTable converter( arIn, arOut );
  while( arIn && arIn.peek() != EOF )
    converter.HandleMessage();
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
StreamToTable::OnParam( istream& arIn )
{
  return Param().ReadBinary( arIn );
}

bool
StreamToTable::OnState( istream& arIn )
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
StreamToTable::OnVisSignalProperties( istream& arIn )
{
  VisSignalProperties v;
  v.ReadBinary( arIn );
  mSignalProperties = v.SignalProperties();
  return true;
}

bool
StreamToTable::OnVisSignal( istream& arIn )
{
  VisSignal v;
  v.ReadBinary( arIn );
  WriteOut( v );
  return true;
}

bool
StreamToTable::OnStateVector( istream& arIn )
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
    Output() << "#";
    mStateNames.clear();
    for( int i = 0; i < mStatelist.Size(); ++i )
      mStateNames.insert( mStatelist[ i ].Name() );
    for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
      Output() << "\t" << *i;
    for( int i = 0; i < inSignal.Channels(); ++i )
      for( int j = 0; j < inSignal.Elements(); ++j )
        Output() << "\tSignal(" << mSignalProperties.ChannelLabels()[i] << "," << mSignalProperties.ElementLabels()[i] << ")";
    Output() << endl;
    mInitialized = true;
  }
  if( inSignal.Properties() != mSignalProperties )
    bcierr << "Ignored signal with inconsistent properties" << endl;
  else
  {
    if( mpStatevector != NULL )
      for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
        Output() << "\t" << mpStatevector->StateValue( i->c_str() );
    else
      for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
        Output() << "\t0";

    for( int i = 0; i < inSignal.Channels(); ++i )
      for( int j = 0; j < inSignal.Elements(); ++j )
        Output() << "\t" << inSignal( i, j );
    Output() << endl;
  }
  mWriteoutPending = false;
}
