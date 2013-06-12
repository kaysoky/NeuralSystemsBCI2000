////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: Classes that represent BCI2000 visualization messages.
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
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GenericVisualization.h"
#include "MessageHandler.h"
#include "Label.h"
#include "HierarchicalLabel.h"
#include "BCIStream.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdlib>

using namespace std;

// Common to all visualization messages.
istream&
VisBase::ReadBinary( istream& is )
{
  int visID = is.get();
  if( visID == SourceID::ExtendedFormat )
  {
    string s;
    getline( is, s, '\0' );
    istringstream iss( s );
    iss >> mVisID;
  }
  else
  {
    ostringstream oss;
    oss << visID;
    mVisID = oss.str();
  }
  ReadBinarySelf( is );
  return is;
}

ostream&
VisBase::WriteBinary( ostream& os ) const
{
  // We use the traditional message format if the visID can be represented
  // as a single byte number.
  bool oldFormat = false;
  int visID = ::atoi( mVisID.c_str() );
  if( visID >= SourceID::min && visID < SourceID::ExtendedFormat )
  {
    ostringstream oss;
    oss << visID;
    if( oss.str() == mVisID )
      oldFormat = true;
  }
  if( oldFormat )
  {
    os.put( visID & 0xff );
  }
  else
  {
    os.put( static_cast<unsigned char>( SourceID::ExtendedFormat ) );
    if( !mVisID.empty() ) // Maintain compatibility with older modules which don't expect an EncodedString here.
      os << mVisID;
    os.put( '\0' );
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

vector<VisCfg>
VisSignalProperties::ToVisCfg() const
{
  // Translation of a VisSignalProperties message into a set of VisCfg messages.
  vector<VisCfg> result;
  const class SignalProperties& s = SignalProperties();

  if( !s.Name().empty() )
    result.push_back( VisCfg( SourceID(), CfgID::WindowTitle, s.Name() ) );

  string channelUnit = s.ChannelUnit().RawToPhysical( s.ChannelUnit().Offset() + 1 );
  result.push_back( VisCfg( SourceID(), CfgID::ChannelUnit, channelUnit ) );

  int numSamples = static_cast<int>( s.ElementUnit().RawMax() - s.ElementUnit().RawMin() + 1 );
  result.push_back( VisCfg( SourceID(), CfgID::NumSamples, numSamples ) );

  if( numSamples > 0 )
  {
    string symbol;
    double value = s.ElementUnit().Gain() * numSamples;
    int magnitude = static_cast<int>( ::log10( ::fabs( value ) ) );
    if( magnitude > 0 )
      --magnitude;
    else if( magnitude < 0 )
      ++magnitude;
    magnitude /= 3;
    if( magnitude < -3 )
      magnitude = -3;
    if( magnitude > 3 )
      magnitude = 3;
    switch( magnitude )
    {
      case -3:
        symbol = "n";
        value /= 1e-9;
        break;
      case -2:
        symbol = "mu";
        value /= 1e-6;
        break;
      case -1:
        symbol = "m";
        value /= 1e-3;
        break;
      case 0:
        break;
      case 1:
        symbol = "k";
        value /= 1e3;
        break;
      case 2:
        symbol = "M";
        value /= 1e6;
        break;
      case 3:
        symbol = "G";
        value /= 1e9;
        break;
    }
    ostringstream oss;
    oss << setprecision( 10 ) << value / numSamples << symbol << s.ElementUnit().Symbol();
    result.push_back( VisCfg( SourceID(), CfgID::SampleUnit, oss.str() ) );
    result.push_back( VisCfg( SourceID(), CfgID::SampleOffset, s.ElementUnit().Offset() ) );
  }

  // Although the SignalProperties class allows for individual units for
  // individual channels, the SignalDisplay class is restricted to a single
  // unit and range.
  string valueUnit = s.ValueUnit().RawToPhysical( s.ValueUnit().Offset() + 1 );
  result.push_back( VisCfg( SourceID(), CfgID::ValueUnit, valueUnit ) );

  double rangeMin = s.ValueUnit().RawMin(),
         rangeMax = s.ValueUnit().RawMax();
  if( rangeMin == rangeMax )
  {
    result.push_back( VisCfg( SourceID(), CfgID::MinValue, "none" ) );
    result.push_back( VisCfg( SourceID(), CfgID::MaxValue, "none" ) );
  }
  else
  {
    result.push_back( VisCfg( SourceID(), CfgID::MinValue, rangeMin ) );
    result.push_back( VisCfg( SourceID(), CfgID::MaxValue, rangeMax ) );
  }

  LabelList groupLabels,
            channelLabels;
  int channelGroupSize = 1;
  if( !s.ChannelLabels().IsTrivial() )
  {
    for( int i = 0; i < s.ChannelLabels().Size(); ++i )
    {
      istringstream iss( s.ChannelLabels()[ i ] );
      HierarchicalLabel label;
      iss >> label;
      if( label.size() == 2 )
      {
        if( groupLabels.empty() )
        {
          groupLabels.push_back( Label( 0, label[ 0 ] ) );
        }
        else
        {
          if( label[ 0 ] == groupLabels.begin()->Text() )
            ++channelGroupSize;
          if( label[ 0 ] != groupLabels.rbegin()->Text() )
            groupLabels.push_back( Label( static_cast<int>( groupLabels.size() ), label[ 0 ] ) );
        }
        channelLabels.push_back( Label( static_cast<int>( channelLabels.size() ), label[ 1 ] ) );
      }
      else
      {
        channelLabels.push_back( Label( i, s.ChannelLabels()[ i ] ) );
      }
    }
  }
  result.push_back( VisCfg( SourceID(), CfgID::ChannelGroupSize, channelGroupSize ) );
  result.push_back( VisCfg( SourceID(), CfgID::ChannelLabels, channelLabels ) );
  result.push_back( VisCfg( SourceID(), CfgID::GroupLabels, groupLabels ) );

  return result;
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

std::ostream* GenericVisualization::spOutputStream = NULL;
const OSMutex* GenericVisualization::spOutputLock = NULL;

GenericVisualization&
GenericVisualization::SendCfgString( CfgID inCfgID, const std::string& inCfgString )
{
  return SendObject( VisCfg( mVisID, inCfgID, inCfgString ) );
}

GenericVisualization&
GenericVisualization::Send( const string& s )
{
  return SendObject( VisMemo( mVisID, s ) );
}

GenericVisualization&
GenericVisualization::Send( const GenericSignal& s )
{
  return SendObject( VisSignal( mVisID, s ) );
}

GenericVisualization&
GenericVisualization::Send( const SignalProperties& s )
{
  return SendObject( VisSignalProperties( mVisID, s ) );
}

GenericVisualization&
GenericVisualization::Send( const BitmapImage& b )
{
  return SendObject( VisBitmap( mVisID, b ) );
}

template<typename T>
GenericVisualization&
GenericVisualization::SendObject( const T& inObject )
{
  if( spOutputStream )
  {
    OSMutex::Lock lock( spOutputLock );
    MessageHandler::PutMessage<T>( *spOutputStream, inObject );
    spOutputStream->flush();
    this->flags( spOutputStream->flags() );
  }
  else
  {
    bcierr << "No output stream specified." << endl;
  }
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
  // An empty image indicates that the next frame is a reference frame.
  Send( BitmapImage( 0, 0 ) ); 
  mImageBuffer = b;
  Send( mImageBuffer );
}

void
BitmapVisualization::SendDifferenceFrame( const BitmapImage& b )
{
  if( b.Width() != mImageBuffer.Width() || b.Height() != mImageBuffer.Height() )
  {
    SendReferenceFrame( b );
  }
  else
  {
    BitmapImage curImage = b;
    Send( curImage - mImageBuffer );
    mImageBuffer = curImage;
  }
}


