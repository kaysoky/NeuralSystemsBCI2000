////////////////////////////////////////////////////////////////////
// $Id$
// File:    bci_stream2asc.cpp
// Date:    Jul 29, 2003
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
// $Log$
// Revision 1.5  2006/01/17 17:39:44  mellinger
// Fixed list of project files.
//
// Revision 1.4  2006/01/12 20:37:14  mellinger
// Adaptation to latest revision of parameter and state related class interfaces.
//
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <typeinfo>

#include "bci_tool.h"
#include "shared/UStatus.h"
#include "shared/UParameter.h"
#include "shared/UState.h"
#include "shared/USysCommand.h"
#include "shared/UGenericVisualization.h"
#include "shared/MessageHandler.h"

using namespace std;

string ToolInfo[] =
{
  "bci_stream2asc",
  "$Revision$, compiled "__DATE__,
  "Convert a binary BCI2000 stream into a human readable form.",
  "Reads a BCI2000 compliant binary stream from standard input, "
    "and writes it to standard output as a sequence of "
    "BCI2000 messages in a human readable representation.",
  ""
};

class StreamToAsc : public MessageHandler
{
 public:
  StreamToAsc( ostream& arOut )
  : mrOut( arOut ), mpStatevector( NULL ) {}
  ~StreamToAsc() { delete mpStatevector; }

 private:
  ostream& mrOut;
  STATELIST mStatelist;
  STATEVECTOR* mpStatevector;

  virtual bool HandleSTATUS(      istream& );
  virtual bool HandlePARAM(       istream& );
  virtual bool HandleSTATE(       istream& );
  virtual bool HandleVisSignal(   istream& );
  virtual bool HandleSTATEVECTOR( istream& );
  virtual bool HandleSYSCMD(      istream& );
};

template<typename T> void Convert( istream&, ostream& );
template<typename T> void Print( ostream&, const T& );

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
  StreamToAsc converter( arOut );
  while( arIn && arIn.peek() != EOF )
    converter.HandleMessage( arIn );
  if( !arIn )
    return illegalInput;
  return noError;
}

template<typename T>
void
Convert( istream& arIn, ostream& arOut )
{
  T t;
  t.ReadBinary( arIn );
  Print( arOut, t );
}

template<typename T>
void
Print( ostream& arOut, const T& arObj )
{
  arOut << typeid( T ).name() << " { "
        << setw( 2 ) << arObj << "\n}\n";
}

bool
StreamToAsc::HandleSTATUS( istream& arIn )
{
  Convert<STATUS>( arIn, mrOut );
  return true;
}

bool
StreamToAsc::HandlePARAM( istream& arIn )
{
  Convert<PARAM>( arIn, mrOut );
  return true;
}

bool
StreamToAsc::HandleSTATE( istream& arIn )
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
    Print( mrOut, s );
  }
  return true;
}

bool
StreamToAsc::HandleVisSignal( istream& arIn )
{
  Convert<VisSignal>( arIn, mrOut );
  return true;
}

bool
StreamToAsc::HandleSTATEVECTOR( istream& arIn )
{
  if( mpStatevector == NULL )
    mpStatevector = new STATEVECTOR( mStatelist, true );
  mpStatevector->ReadBinary( arIn );
  Print( mrOut, *mpStatevector );
  return true;
}

bool
StreamToAsc::HandleSYSCMD( istream& arIn )
{
  Convert<SYSCMD>( arIn, mrOut );
  return true;
}
