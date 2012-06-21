////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: GenericSignal is the BCI2000 type representing filter input and
//              output data.
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
#ifndef GENERIC_SIGNAL_H
#define GENERIC_SIGNAL_H

#include <vector>
#include <iostream>
#include "SignalType.h"
#include "SignalProperties.h"

#ifdef __GNUC__
# define NOINLINE  __attribute__ ((noinline))
#else
# define NOINLINE
#endif // __GNUC__

class GenericChannel;
class GenericElement;

class GenericSignal
{
  public:
    typedef double ValueType;

    GenericSignal();
    ~GenericSignal();
    GenericSignal( const GenericSignal& );
    GenericSignal& operator=( const GenericSignal& );

    GenericSignal(
      size_t inChannels,
      size_t inMaxElements,
      SignalType::Type inType = SignalType::defaultType );
    GenericSignal(
      size_t inChannels,
      size_t inMaxElements,
      SignalType inType );
    explicit GenericSignal( const SignalProperties& );
    GenericSignal( const SignalProperties&, ValueType );

    GenericSignal&          SetProperties( const SignalProperties& );
    const SignalProperties& Properties() const
                            { return mProperties; }

    // Read access to properties
    int               Channels() const
                      { return mProperties.Channels(); }
    int               Elements() const
                      { return mProperties.Elements(); }
    const SignalType& Type() const
                      { return mProperties.Type(); }

    // Value Accessors
    const ValueType&  Value( size_t inChannel, size_t inElement ) const;
    GenericSignal&    SetValue( size_t inChannel, size_t inElement, ValueType inValue );
    GenericSignal&    AssignValues( const GenericSignal& );
    // Read access
    const ValueType&  operator() ( size_t inChannel, size_t inElement ) const;
    // Write access
    ValueType&        operator() ( size_t inChannel, size_t inElement );

    // Stream i/o
    std::ostream&     WriteToStream( std::ostream& ) const;
    std::istream&     ReadFromStream( std::istream& );
    std::ostream&     WriteValueBinary( std::ostream&, size_t inChannel, size_t inElement ) const;
    std::istream&     ReadValueBinary( std::istream&, size_t inChannel, size_t inElement );
    std::ostream&     WriteBinary( std::ostream& ) const;
    std::istream&     ReadBinary( std::istream& );

    template<SignalType::Type>
      void PutValueBinary( std::ostream&, size_t inChannel, size_t inElement ) const;
    template<SignalType::Type>
      void GetValueBinary( std::istream&, size_t inChannel, size_t inElement );

  private:
    template<typename T> static void PutLittleEndian( std::ostream&, const T& );
    template<typename T> static void GetLittleEndian( std::istream&, T& );

  private:
    SignalProperties mProperties;
    ValueType* mpValues;
};

class GenericChannel
{
  public:
    GenericChannel( GenericSignal& s, int ch ) : mrSignal( s ), mCh( ch ) {}
    GenericChannel& operator=( const GenericChannel& );
    int Channels() const { return 1; }
    int Elements() const { return mrSignal.Elements(); }
    size_t size() const { return Elements(); }
    GenericSignal::ValueType& operator[]( size_t el ) { return mrSignal( mCh, el ); }
    GenericSignal::ValueType& operator()( int ch, int el ) { return ( *this )[el]; }
    const GenericSignal::ValueType& operator[]( size_t el ) const { return mrSignal( mCh, el ); }
    const GenericSignal::ValueType& operator()( int ch, int el ) const { return ( *this )[el]; }
  private:
    GenericSignal& mrSignal;
    int mCh;
};

class GenericElement
{
  public:
    GenericElement( GenericSignal& s, int el ) : mrSignal( s ), mEl( el ) {}
    GenericElement& operator=( const GenericElement& );
    int Channels() const { return mrSignal.Channels(); }
    int Elements() const { return 1; }
    size_t size() const { return Channels(); }
    GenericSignal::ValueType& operator[]( size_t ch ) { return mrSignal( ch, mEl ); }
    GenericSignal::ValueType& operator()( int ch, int el ) { return ( *this )[ch]; }
    const GenericSignal::ValueType& operator[]( size_t ch ) const { return mrSignal( ch, mEl ); }
    const GenericSignal::ValueType& operator()( int ch, int el ) const { return ( *this )[ch]; }
  private:
    GenericSignal& mrSignal;
    int mEl;
};

template<> void
GenericSignal::PutValueBinary<SignalType::int16>( std::ostream&, size_t, size_t ) const;
template<> void
GenericSignal::PutValueBinary<SignalType::int32>( std::ostream&, size_t, size_t ) const;
template<> void
GenericSignal::PutValueBinary<SignalType::float24>( std::ostream&, size_t, size_t ) const;
template<> void
GenericSignal::PutValueBinary<SignalType::float32>( std::ostream&, size_t, size_t ) const;

template<> void
GenericSignal::GetValueBinary<SignalType::int16>( std::istream&, size_t, size_t );
template<> void
GenericSignal::GetValueBinary<SignalType::int32>( std::istream&, size_t, size_t );
template<> void
GenericSignal::GetValueBinary<SignalType::float24>( std::istream&, size_t, size_t );
template<> void
GenericSignal::GetValueBinary<SignalType::float32>( std::istream&, size_t, size_t );

inline
const GenericSignal::ValueType&
GenericSignal::Value( size_t inChannel, size_t inElement ) const
{
  return operator()( inChannel, inElement );
}

inline
GenericSignal&
GenericSignal::SetValue( size_t inChannel, size_t inElement, ValueType inValue )
{
  operator()( inChannel, inElement ) = inValue;
  return *this;
}

inline
const GenericSignal::ValueType&
GenericSignal::operator() ( size_t inChannel, size_t inElement ) const
{
  return mpValues[inChannel * mProperties.Elements() + inElement];
}

inline
GenericSignal::ValueType&
GenericSignal::operator() ( size_t inChannel, size_t inElement )
{
  return mpValues[inChannel * mProperties.Elements() + inElement];
}

inline
std::ostream& operator<<( std::ostream& os, const GenericSignal& s )
{
  return s.WriteToStream( os );
}

inline
std::istream& operator>>( std::istream& is, GenericSignal& s )
{
  return s.ReadFromStream( is );
}

#endif // GENERIC_SIGNAL_H

