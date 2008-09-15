////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents properties of a GenericSignal.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SIGNAL_PROPERTIES_H
#define SIGNAL_PROPERTIES_H

#include <iostream>
#include "SignalType.h"
#include "PhysicalUnit.h"
#include "LabelIndex.h"
#include "ValueList.h"
#include "EncodedString.h"

class SignalProperties
{
  public:
    SignalProperties()
      : mType( SignalType::defaultType ),
        mValueUnits( 1 )
      { InitMembers( 0, 0 ); }

    SignalProperties(
      size_t inChannels,
      size_t inElements,
      SignalType::Type inType = SignalType::defaultType )
      : mType( inType ),
        mValueUnits( 1 )
      { InitMembers( inChannels, inElements ); }

    SignalProperties(
      size_t inChannels,
      size_t inElements,
      SignalType inType )
      : mType( inType ),
        mValueUnits( 1 )
      { InitMembers( inChannels, inElements ); }

    virtual ~SignalProperties()
      {}

    // Accessors
    SignalProperties& SetChannels( size_t ch )
                      { mChannelLabels.Resize( ch ); return *this; }
    int               Channels() const
                      { return mChannelLabels.Size(); }

    SignalProperties& SetElements( size_t el )
                      { mElementLabels.Resize( el ); return *this; }
    int               Elements() const
                      { return mElementLabels.Size(); }

    SignalProperties& SetType( const SignalType& ty )
                      { mType = ty; return *this; }
    const SignalType& Type() const
                      { return mType; }

    SignalProperties& SetName( const std::string& s )
                      { mName = s; return *this; }
    const std::string& Name() const
                      { return mName; }

    LabelIndex&       ChannelLabels()
                      { return mChannelLabels; }
    const LabelIndex& ChannelLabels() const
                      { return mChannelLabels; }
    PhysicalUnit&     ChannelUnit()
                      { return mChannelUnit; }
    const PhysicalUnit& ChannelUnit() const
                      { return mChannelUnit; }
    // Convert a string-valued channel address into a numeric index.
    // The address may be a label or a value in physical units.
    float             ChannelIndex( const std::string& address ) const
                      { return AddressToIndex( address, ChannelLabels(), ChannelUnit() ); }

    LabelIndex&       ElementLabels()
                      { return mElementLabels; }
    const LabelIndex& ElementLabels() const
                      { return mElementLabels; }
    PhysicalUnit&     ElementUnit()
                      { return mElementUnit; }
    const PhysicalUnit& ElementUnit() const
                      { return mElementUnit; }
    // Convert a string-valued element address into a numeric index.
    // The address may be a label or a value in physical units.
    float             ElementIndex( const std::string& address ) const
                      { return AddressToIndex( address, ElementLabels(), ElementUnit() ); }

    PhysicalUnit&     ValueUnit( size_t ch = 0 );
    const PhysicalUnit& ValueUnit( size_t ch = 0 ) const
                      { return mValueUnits.size() <= ch
                        ? *mValueUnits.rbegin() : mValueUnits[ ch ]; }

    bool IsEmpty() const
                      { return Channels() == 0 || Elements() == 0; }

    bool operator==( const SignalProperties& sp ) const
                      { return Type() == sp.Type() && Elements() == sp.Elements() && Channels() == sp.Channels(); }
    bool operator!=( const SignalProperties& sp ) const
                      { return !( *this == sp ); }
    bool Accommodates( const SignalProperties& sp ) const;

    // Stream i/o
    std::ostream& WriteToStream( std::ostream& ) const;
    std::istream& ReadFromStream( std::istream& );

  private:
    void  InitMembers( int numChannels, int numElements );
    float AddressToIndex( const std::string&, const LabelIndex&, const PhysicalUnit& ) const;

    EncodedString                      mName;
    LabelIndex                         mChannelLabels,
                                       mElementLabels;
    SignalType                         mType;
    PhysicalUnit                       mChannelUnit,
                                       mElementUnit;
    ValueList<PhysicalUnit>            mValueUnits;
};

inline
std::ostream& operator<<( std::ostream& os, const SignalProperties& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, SignalProperties& s )
{ return s.ReadFromStream( is );
}
#endif // SIGNAL_PROPERTIES_H

