////////////////////////////////////////////////////////////////////
// $Id$
// File:    bci_stream2table.cpp
// Date:    Jan 13, 2005
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
// $Log$
// Revision 1.7  2006/01/12 20:37:14  mellinger
// Adaptation to latest revision of parameter and state related class interfaces.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
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
  "bci_stream2table",
  "$Revision$, compiled "__DATE__,
  "Convert a binary BCI2000 stream into a human readable tabular form.",
  "Reads a BCI2000 compliant binary stream from standard input, "
    "and writes it to standard output "
    "as a tab/newline separated table of values.",
  ""
};

class StreamToTable : public MessageHandler
{
 public:
  StreamToTable( ostream& arOut )
  : mrOut( arOut ), mpStatevector( NULL ), mSignalProperties( 0, 0 ) {}
  ~StreamToTable() { delete mpStatevector; }

 private:
  ostream&            mrOut;
  STATELIST           mStatelist;
  STATEVECTOR*        mpStatevector;
  SignalProperties    mSignalProperties;
  typedef set<string> StringSet; // A set is a sorted container of unique values.
  StringSet           mStateNames;

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
StreamToTable::HandleSTATE( istream& arIn )
{
  STATE s;
  s.ReadBinary( arIn );
  if( arIn )
  {
    mStatelist.Delete( s.GetName() );
    mStatelist.Add( s );
    if( mpStatevector != NULL )
    {
      delete mpStatevector;
      mpStatevector = new STATEVECTOR( mStatelist, true );
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
    mSignalProperties = s.GetProperties();
    mrOut << "#";
    mStateNames.clear();
    for( size_t i = 0; i < mStatelist.Size(); ++i )
      mStateNames.insert( mStatelist[ i ].GetName() );
    for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
      mrOut << "\t" << *i;
    for( size_t i = 0; i < s.Channels(); ++i )
      for( size_t j = 0; j < s.Elements(); ++j )
        mrOut << "\tSignal(" << i << "," << j << ")";
    mrOut << endl;
  }
  if( s.GetProperties() != mSignalProperties )
    bcierr << "Ignored signal with inconsistent properties" << endl;
  else
  {
    if( mpStatevector != NULL )
      for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
        mrOut << "\t" << mpStatevector->GetStateValue( i->c_str() );
    else
      for( StringSet::const_iterator i = mStateNames.begin(); i != mStateNames.end(); ++i )
        mrOut << "\t0";

    for( size_t i = 0; i < s.Channels(); ++i )
      for( size_t j = 0; j < s.Elements(); ++j )
        mrOut << "\t" << s( i, j );
    mrOut << endl;
  }
  return true;
}

bool
StreamToTable::HandleSTATEVECTOR( istream& arIn )
{
  if( mpStatevector == NULL )
    mpStatevector = new STATEVECTOR( mStatelist, true );
  mpStatevector->ReadBinary( arIn );
  return true;
}
