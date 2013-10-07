////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents properties of a GenericSignal.
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

#include "SignalProperties.h"
#include "LengthField.h"
#include "BCIAssert.h"
#include <sstream>

using namespace std;

SignalProperties::ValueUnitsProxy
SignalProperties::ValueUnit()
{
  return ValueUnitsProxy( this );
}

PhysicalUnit&
SignalProperties::ValueUnit( size_t inCh )
{
  if( inCh >= mValueUnits.size() && int( inCh ) < Channels() )
    mValueUnits.resize( inCh + 1, *mValueUnits.rbegin() );
  return mValueUnits[ min( int( inCh ), Channels() ) ];
}

ostream&
SignalProperties::WriteToStream( ostream& os ) const
{
  return os << mName << " "
            << mChannelLabels << " "
            << mElementLabels << " "
            << mType << " "
            << mChannelUnit << " "
            << mElementUnit << " "
            << mValueUnits;
}

istream&
SignalProperties::ReadFromStream( istream& is )
{
  return is >> mName
            >> mChannelLabels
            >> mElementLabels
            >> mType
            >> mChannelUnit
            >> mElementUnit
            >> mValueUnits;
}

bool
SignalProperties::Accommodates( const SignalProperties& sp ) const
{
  if( sp.IsEmpty() )
    return true;
  if( IsEmpty() )
    return false;
  if( !SignalType::ConversionIsSafe( sp.Type(), Type() ) )
    return false;
  if( Elements() < sp.Elements() )
    return false;
  if( Elements() < sp.Elements() )
    return false;
  return true;
}

double 
SignalProperties::ChannelIndex( const std::string& address ) const
{
  double idx = ChannelLabels().AddressToIndex( address, ChannelUnit() );
  return idx >= Channels() ? -1 : idx;

}

double 
SignalProperties::ElementIndex( const std::string& address ) const
{
  double idx = ElementLabels().AddressToIndex( address, ElementUnit() );
  return idx >= Elements() ? -1 : idx;
}

size_t
SignalProperties::LinearIndex( size_t ch, size_t el ) const
{
  if( ch >= size_t( Channels() ) )
    bcidebug( "Channel index out of bounds" );
  if( el >= size_t( Elements() ) )
    bcidebug( "Element index out of bounds" );
  return ch * Elements() + el;
}

void
SignalProperties::InitMembers( size_t inChannels, size_t inElements )
{
  SetChannels( inChannels );
  SetElements( inElements );
  ChannelUnit().SetOffset( 0 )
               .SetGain( 1 )
               .SetSymbol( "" )
               .SetRawMin( 0 )
               .SetRawMax( static_cast<int>( inChannels ) - 1 );
  ElementUnit().SetOffset( 0 )
               .SetGain( 1 )
               .SetSymbol( "" )
               .SetRawMin( 0 )
               .SetRawMax( static_cast<int>( inElements ) - 1 );
  ChannelLabels().Resize( inChannels );
  ElementLabels().Resize( inElements );
  SetUpdateRate( 0.0 );
}

double
SignalProperties::SamplingRate() const
{
  if( mElementUnit.Symbol() == "s" )
    return 1.0 / mElementUnit.Gain();
  if( Elements() == 1 )
    return mUpdateRate;
  return 0.0;
}

bool
SignalProperties::IsStream() const
{
  if( mIsStream == true_ )
    return true;
  if( mIsStream == false_ )
    return false;
  return Elements() == 1 || ElementUnit().Symbol() == "s";
}

// ValueUnitsProxy
SignalProperties::ValueUnitsProxy::ValueUnitsProxy( SignalProperties* p )
: PhysicalUnit( p->ValueUnit( 0 ) ),
  p( p )
{
}

SignalProperties::ValueUnitsProxy::ValueUnitsProxy( ValueUnitsProxy& v )
: PhysicalUnit( v ),
  p( v.p )
{
  v.p = 0;
}

SignalProperties::ValueUnitsProxy::~ValueUnitsProxy()
{
  if( p )
    for( int ch = 0; ch < p->Channels(); ++ch )
      p->ValueUnit( ch ) = *this;
}
