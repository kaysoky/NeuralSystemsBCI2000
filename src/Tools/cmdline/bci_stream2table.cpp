////////////////////////////////////////////////////////////////////
// File:    bci_stream2table.cpp
// Date:    Jan 13, 2005
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
////////////////////////////////////////////////////////////////////
#include <iostream>

#include "bci_tool.h"
#include "shared/UParameter.h"
#include "shared/UState.h"
#include "shared/UGenericVisualization.h"
#include "shared/MessageHandler.h"

using namespace std;

string ToolInfo[] =
{
  "bci_stream2table",
  "version 0.1.0, compiled "__DATE__,
  "Convert a binary BCI2000 stream into a human readable tabular form.",
  "Reads a BCI2000 compliant binary stream from standard input, "
    "and writes it output to the standard "
    "output as a tab separated table of values.",
  ""
};

class StreamToTable : public MessageHandler
{
 public:
  StreamToTable( ostream& arOut )
  : mrOut( arOut ), mpStatevector( NULL ), mSignalProperties( 0, 0, 0 ) {}
  ~StreamToTable() { delete mpStatevector; }

 private:
  ostream& mrOut;
  STATELIST mStatelist;
  STATEVECTOR* mpStatevector;
  SignalProperties mSignalProperties;

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
StreamToTable::HandleVisSignal( istream& arIn )
{
  VisSignal v;
  v.ReadBinary( arIn );
  const GenericSignal& s = v;
  // Print a header line before the first data.
  if( mSignalProperties == SignalProperties( 0, 0, 0 ) )
  {
    mSignalProperties = s;
    mrOut << "#";
    for( int i = 0; i < mStatelist.GetNumStates(); ++i )
      mrOut << "\t" << mStatelist.GetStatePtr( i )->GetName();
    for( size_t i = 0; i < s.Channels(); ++i )
      for( size_t j = 0; j < s.GetNumElements( i ); ++j )
        mrOut << "\tSignal(" << i << "," << j << ")";
    mrOut << endl;
  }
  if( s != mSignalProperties )
    bcierr << "Ignored signal with inconsistent properties" << endl;
  else
  {
    for( int i = 0; i < mStatelist.GetNumStates(); ++i )
      mrOut << "\t" << mpStatevector->GetStateValue( mStatelist.GetStatePtr( i )->GetName() );
    for( size_t i = 0; i < s.Channels(); ++i )
      for( size_t j = 0; j < s.GetNumElements( i ); ++j )
        mrOut << "\t" << s( i, j );
    mrOut << endl;
  }
  return true;
}

bool
StreamToTable::HandleSTATEVECTOR( istream& arIn )
{
  if( mpStatevector == NULL )
    mpStatevector = new STATEVECTOR( &mStatelist, true );
  mpStatevector->ReadBinary( arIn );
  return true;
}
