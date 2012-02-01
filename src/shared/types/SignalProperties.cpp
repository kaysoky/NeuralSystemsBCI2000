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
#include <sstream>

using namespace std;

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


double
SignalProperties::AddressToIndex( const string& inAddress,
                                  const LabelIndex& inLabelIndex,
                                  const PhysicalUnit& inUnit ) const
{
  double result = -1;
  // Does the address match an existing label?
  if( inLabelIndex.Exists( inAddress ) )
    result = inLabelIndex[ inAddress ];
  // Is it a value in physical units?
  else if( inUnit.IsPhysical( inAddress ) )
    result = inUnit.PhysicalToRaw( inAddress );
  // If neither, then we interpret it as a 1-based index.
  else
  {
    istringstream is( inAddress );
    double number;
    if( is >> number && is.eof() )
      result = number - 1;
  }
  return result;
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

