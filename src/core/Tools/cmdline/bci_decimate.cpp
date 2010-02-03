////////////////////////////////////////////////////////////////////
// $Id$
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <typeinfo>

#include "bci_tool.h"
#include "ClassName.h"
#include "ProtocolVersion.h"
#include "Status.h"
#include "Param.h"
#include "ParamList.h"
#include "State.h"
#include "StateList.h"
#include "StateVector.h"
#include "SysCommand.h"
#include "GenericVisualization.h"
#include "MessageHandler.h"
#include "Version.h"

using namespace std;
using namespace bci;

string ToolInfo[] =
{
  "bci_decimate",
  BCI2000_VERSION,
  "Decimate data in a binary BCI2000 stream.",
  "Reads a BCI2000 compliant binary stream from standard input, "
    "and writes it to standard output, retaining only every "
    "Nth statevector and signal message.",
  "-d<N>,    --decimate<N> \tDecimation factor, defaults to 1",
  ""
};

class Decimate : public MessageHandler
{
 public:
  Decimate( int factor, ostream& arOut )
  : mFactor( factor ), mCount( 0 ), mrOut( arOut ), mpStatevector( NULL ) {}
  ~Decimate() { delete mpStatevector; }

 private:
  int mFactor,
      mCount;
  ostream& mrOut;
  StateList mStatelist;
  StateVector* mpStatevector;

  virtual bool HandleProtocolVersion(     istream& );
  virtual bool HandleStatus(              istream& );
  virtual bool HandleParam(               istream& );
  virtual bool HandleState(               istream& );
  virtual bool HandleVisSignalProperties( istream& );
  virtual bool HandleVisSignal(           istream& );
  virtual bool HandleStateVector(         istream& );
  virtual bool HandleSysCommand(          istream& );
};

template<typename T> void Forward( istream&, ostream& );

ToolResult
ToolInit()
{
  return noError;
}

ToolResult
ToolMain( const OptionSet& arOptions, istream& arIn, ostream& arOut )
{
  int decimation = ::atoi( arOptions.getopt( "-d|-D|--decimation", "1" ).c_str() );
  Decimate decimator( decimation, arOut );
  while( arIn && arIn.peek() != EOF )
    decimator.HandleMessage( arIn );
  if( !arIn )
    return illegalInput;
  return noError;
}

template<typename T>
void
Forward( istream& arIn, ostream& arOut )
{
  T t;
  t.ReadBinary( arIn );
  MessageHandler::PutMessage( arOut, t );
}

template<typename T>
void
Absorb( istream& arIn )
{
  T t;
  t.ReadBinary( arIn );
}

bool
Decimate::HandleProtocolVersion( istream& arIn )
{
  Forward<ProtocolVersion>( arIn, mrOut );
  return true;
}

bool
Decimate::HandleStatus( istream& arIn )
{
  Forward<Status>( arIn, mrOut );
  return true;
}

bool
Decimate::HandleParam( istream& arIn )
{
  Forward<Param>( arIn, mrOut );
  return true;
}

bool
Decimate::HandleState( istream& arIn )
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
    MessageHandler::PutMessage( mrOut, s );
  }
  return true;
}

bool
Decimate::HandleVisSignalProperties( istream& arIn )
{
  Forward<VisSignalProperties>( arIn, mrOut );
  return true;
}

bool
Decimate::HandleVisSignal( istream& arIn )
{
  if( mCount % mFactor == 0 )
    Forward<VisSignal>( arIn, mrOut );
  else
    Absorb<VisSignal>( arIn );
  return true;
}

bool
Decimate::HandleStateVector( istream& arIn )
{
  if( mpStatevector == NULL )
    mpStatevector = new StateVector( mStatelist );
  mpStatevector->ReadBinary( arIn );
  // state vectors are sent first, so we increase
  // the count here rather than in the signal handler
  if( ( ++mCount %= mFactor ) == 0 )
    MessageHandler::PutMessage( mrOut, *mpStatevector );
  return true;
}

bool
Decimate::HandleSysCommand( istream& arIn )
{
  Forward<SysCommand>( arIn, mrOut );
  return true;
}
