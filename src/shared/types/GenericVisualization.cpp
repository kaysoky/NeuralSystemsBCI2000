////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: Classes that represent BCI2000 visualization messages.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GenericVisualization.h"

#include "Environment.h"
#include "GenericSignal.h"
#include "MessageHandler.h"

#include <iostream>
#include <string>
#include <sstream>

using namespace std;

// Common to all visualization messages.
istream&
VisBase::ReadBinary( istream& is )
{
  int sourceID = is.get();
  if( sourceID == InvalidID )
  {
    getline( is, mSourceID, '\0' );
  }
  else
  {
    ostringstream oss;
    oss << sourceID;
    mSourceID = oss.str();
  }
  ReadBinarySelf( is );
  return is;
}

ostream&
VisBase::WriteBinary( ostream& os ) const
{
  // We use the traditional message format if the source ID can be represented
  // as a single byte number.
  bool oldFormat = false;
  int sourceID = ::atoi( mSourceID.c_str() );
  if( sourceID >= 52 && sourceID < 255 )
  {
    ostringstream oss;
    oss << sourceID;
    if( oss.str() == mSourceID )
      oldFormat = true;
  }
  if( oldFormat )
  {
    os.put( sourceID & 0xff );
  }
  else
  {
    os.put( 0xff )
      .write( mSourceID.data(), mSourceID.length() )
      .put( '\0' );
  }
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

// Signal properties message.
void
VisSignalProperties::ReadBinarySelf( istream& is )
{
  mSignalProperties.ReadFromStream( is );
}

void
VisSignalProperties::WriteBinarySelf( ostream& os ) const
{
  mSignalProperties.WriteToStream( os );
}

// Bitmap message.
void
VisBitmap::ReadBinarySelf( istream& is )
{
  mBitmap.ReadBinary( is );
}

void
VisBitmap::WriteBinarySelf( ostream& os ) const
{
  mBitmap.WriteBinary( os );
}

template<>
GenericVisualization&
GenericVisualization::Send( CfgID::CfgID inCfgID, const string& inCfgString )
{
  MessageHandler::PutMessage( *EnvironmentBase::Operator, VisCfg( mSourceID, inCfgID, inCfgString ) );
  EnvironmentBase::Operator->flush();
  this->flags( EnvironmentBase::Operator->flags() );
  return *this;
}

GenericVisualization&
GenericVisualization::Send( const string& s )
{
  MessageHandler::PutMessage( *EnvironmentBase::Operator, VisMemo( mSourceID, s ) );
  EnvironmentBase::Operator->flush();
  this->flags( EnvironmentBase::Operator->flags() );
  return *this;
}

GenericVisualization&
GenericVisualization::Send( const GenericSignal& s )
{
  MessageHandler::PutMessage( *EnvironmentBase::Operator, VisSignal( mSourceID, s ) );
  EnvironmentBase::Operator->flush();
  this->flags( EnvironmentBase::Operator->flags() );
  return *this;
}

GenericVisualization&
GenericVisualization::Send( const SignalProperties& s )
{
  MessageHandler::PutMessage( *EnvironmentBase::Operator, VisSignalProperties( mSourceID, s ) );
  EnvironmentBase::Operator->flush();
  this->flags( EnvironmentBase::Operator->flags() );
  return *this;
}

GenericVisualization&
GenericVisualization::Send( const BitmapImage& b )
{
  MessageHandler::PutMessage( *EnvironmentBase::Operator, VisBitmap( mSourceID, b ) );
  EnvironmentBase::Operator->flush();
  this->flags( EnvironmentBase::Operator->flags() );
  return *this;
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


