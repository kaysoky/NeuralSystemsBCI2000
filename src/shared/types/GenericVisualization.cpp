////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: Classes that represent BCI2000 visualization messages.
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
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GenericVisualization.h"
#include "Environment.h"
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
    os.put( '\xff' )
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

GenericVisualization&
GenericVisualization::SendCfgString( CfgID::CfgID inCfgID, const std::string& inCfgString )
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

void
BitmapVisualization::SendReferenceFrame( const BitmapImage& b )
{
  // An empty image indicates that the next frame is a reference frame
  Send( BitmapImage( 0, 0 ) ); 
  mImageBuffer = b;
  Send( mImageBuffer - BitmapImage( mImageBuffer.Width(), mImageBuffer.Height() ) );
}

void
BitmapVisualization::SendDifferenceFrame( const BitmapImage& b )
{
  BitmapImage curImage = b;
  Send( curImage - mImageBuffer );
  mImageBuffer = curImage;
}


