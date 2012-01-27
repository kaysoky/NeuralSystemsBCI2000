////////////////////////////////////////////////////////////////////
// $Id$
// Author:  juergen.mellinger@uni-tuebingen.de
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
  : mrOut( arOut ), mpStatevector( NULL ), mSignalProperties( 0, 0 ) {}
  ~StreamToTable() { delete mpStatevector; }

 private:
  ostream&            mrOut;
  StateList           mStatelist;
  StateVector*        mpStatevector;
  SignalProperties    mSignalProperties;
  typedef set<string> StringSet; // A set is a sorted container of unique values.
  StringSet           mStateNames;

  virtual bool HandleState(       istream& );
  virtual bool HandleVisSignal(   istream& );
  virtual bool HandleStateVector( istream& );
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
  if( !arIn )
    return illegalInput;
  return noError;
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
StreamToTable::HandleVisSignal( istream& arIn )
{
  VisSignal v;
  v.ReadBinary( arIn );
  const GenericSignal& s = v;
  // Print a header line before the first line of data.
  if( mSignalProperties.IsEmpty() )
  {
    mSignalProperties = s.Properties();
    mrOut << "#";
    mStateNames.clear();
    for( int i = 0; i < mStatelist.Size(); ++i )
      mStateNames.insert( mStatelist[ i ].Name() );
    for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
      mrOut << "\t" << *i;
    for( int i = 0; i < s.Channels(); ++i )
      for( int j = 0; j < s.Elements(); ++j )
        mrOut << "\tSignal(" << i << "," << j << ")";
    mrOut << endl;
  }
  if( s.Properties() != mSignalProperties )
    bcierr << "Ignored signal with inconsistent properties" << endl;
  else
  {
    if( mpStatevector != NULL )
      for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
        mrOut << "\t" << mpStatevector->StateValue( i->c_str() );
    else
      for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
        mrOut << "\t0";

    for( int i = 0; i < s.Channels(); ++i )
      for( int j = 0; j < s.Elements(); ++j )
        mrOut << "\t" << s( i, j );
    mrOut << endl;
  }
  return true;
}

bool
StreamToTable::HandleStateVector( istream& arIn )
{
  if( mpStatevector == NULL )
    mpStatevector = new StateVector( mStatelist );
  mpStatevector->ReadBinary( arIn );
  return true;
}
