////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File:    UGenericVisualization.cpp
//
// Authors: Gerwin Schalk, Juergen Mellinger
//
// Changes: Apr 15, 2003, juergen.mellinger@uni-tuebingen.de:
//          Reworked graph display double buffering scheme.
//          Untangled window painting from content changes.
//          Introduced clipping to reduce the amount of time spent blitting
//          graphics data.
//
//          May 27, 2003, jm:
//          Separated VISUAL and VISCFGLIST into a file belonging to
//          the operator module.
//
//          Dec 10, 2003, jm:
//          Introduced a RGB color type that reads and writes itself to a
//          stream to allow for transmitting color lists for
//          CFGID::channelColors.
//
//          May 28, 2004, jm:
//          Introduced Memo, Signal, and Cfg objects to allow for centralization
//          of message processing in the MessageHandler class.
// $Log$
// Revision 1.20  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UGenericVisualization.h"

#include "UEnvironment.h"
#include "UGenericSignal.h"
#include "MessageHandler.h"

#include <iostream>
#include <string>
#include <sstream>

using namespace std;

// Common to all visualization messages.
istream&
VisBase::ReadBinary( istream& is )
{
  mSourceID = is.get();
  ReadBinarySelf( is );
  return is;
}

ostream&
VisBase::WriteBinary( ostream& os ) const
{
  os.put( mSourceID & 0xff );
  WriteBinarySelf( os );
  return os;
}

// Config message.
void
VisCfg::ReadBinarySelf( istream& is )
{
  mCfgID = is.get();
  getline( is, mCfgValue, '\0' );
}

void
VisCfg::WriteBinarySelf( ostream& os ) const
{
  os.put( mCfgID );
  os.write( mCfgValue.data(), mCfgValue.length() );
  os.put( '\0' );
}

// Memo message.
void
VisMemo::ReadBinarySelf( istream& is )
{
  getline( is, mMemo, '\0' );
}

void
VisMemo::WriteBinarySelf( ostream& os ) const
{
  os.write( mMemo.data(), mMemo.length() );
  os.put( '\0' );
}

// Signal message.
void
VisSignal::ReadBinarySelf( istream& is )
{
  mSignal.ReadBinary( is );
}

void
VisSignal::WriteBinarySelf( ostream& os ) const
{
  mSignal.WriteBinary( os );
}

template<>
bool
GenericVisualization::Send( CFGID::CFGID inCfgID, const string& inCfgString )
{
  MessageHandler::PutMessage( *Environment::Operator, VisCfg( mSourceID, inCfgID, inCfgString ) );
  Environment::Operator->flush();
  return *Environment::Operator;
}

bool
GenericVisualization::Send( const string& s )
{
  MessageHandler::PutMessage( *Environment::Operator, VisMemo( mSourceID, s ) );
  Environment::Operator->flush();
  return *Environment::Operator;
}

bool
GenericVisualization::Send( const GenericSignal* s )
{
  MessageHandler::PutMessage( *Environment::Operator, VisSignal( mSourceID, *s ) );
  Environment::Operator->flush();
  return *Environment::Operator;
}

bool
GenericVisualization::SendCfg2Operator( int sourceID, int cfgID, int cfgValue )
{
  ostringstream oss;
  oss << cfgValue;
  return SendCfg2Operator( sourceID, cfgID, oss.str().c_str() );
}

bool
GenericVisualization::SendCfg2Operator( int sourceID, int cfgID, const char* cfgString )
{
  if( mSourceID == invalidID )
    mSourceID = sourceID;
  else if( sourceID != mSourceID )
    bcierr << "Source ID parameter disagrees with class instance's source ID member"
           << endl;
  return Send( static_cast<CFGID::CFGID>( cfgID ), cfgString );
}

int
GenericVisualization::VisStringbuf::sync()
{
  int result = stringbuf::sync();
  if( !mpParent->Send( str() ) )
    mpParent->setstate( ios::failbit );
  str( "" );
  return result;
}

