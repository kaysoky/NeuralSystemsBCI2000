////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents properties of a GenericSignal.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SignalProperties.h"
#include "LengthField.h"
#include "BCIError.h"
#include <sstream>

using namespace std;

PhysicalUnit&
SignalProperties::ValueUnit( size_t inCh )
{
  if( inCh > mValueUnits.size() && int( inCh ) < Channels() )
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
SignalProperties::InitMembers( int inChannels, int inElements )
{
  SetChannels( inChannels );
  SetElements( inElements );
  ChannelUnit().SetOffset( 0 )
               .SetGain( 1 )
               .SetSymbol( "" )
               .SetRawMin( 0 )
               .SetRawMax( inChannels - 1 );
  ElementUnit().SetOffset( 0 )
               .SetGain( 1 )
               .SetSymbol( "" )
               .SetRawMin( 0 )
               .SetRawMax( inElements - 1 );
  ChannelLabels().Resize( inChannels );
  ElementLabels().Resize( inElements );
}

float
SignalProperties::AddressToIndex( const string& inAddress,
                                  const LabelIndex& inLabelIndex,
                                  const PhysicalUnit& inUnit ) const
{
  float result = -1;
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
    float number;
    if( is >> number && is.eof() )
      result = number - 1;
  }
  return result;
}

