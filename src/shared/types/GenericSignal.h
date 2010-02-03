////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: GenericSignal is the BCI2000 type representing filter input and
//              output data.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GENERIC_SIGNAL_H
#define GENERIC_SIGNAL_H

#include <vector>
#include <iostream>
#include "SignalType.h"
#include "SignalProperties.h"

class GenericSignal
{
  public:
    typedef double ValueType;

    GenericSignal();
    GenericSignal(
      size_t inChannels,
      size_t inMaxElements,
      SignalType::Type inType = SignalType::defaultType );
    GenericSignal(
      size_t inChannels,
      size_t inMaxElements,
      SignalType inType );
    explicit GenericSignal( const SignalProperties& );

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
    // Bracket read access
    const ValueType&  operator() ( size_t inChannel, size_t inElement ) const;
    // Bracket write access
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
    SignalProperties                     mProperties;
    std::vector<std::vector<ValueType> > mValues;
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
std::ostream& operator<<( std::ostream& os, const GenericSignal& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, GenericSignal& s )
{ return s.ReadFromStream( is ); }

#endif // GENERIC_SIGNAL_H

