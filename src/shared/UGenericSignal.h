////////////////////////////////////////////////////////////////////////////////
//
// File: UGenericSignal.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: This file declares a SignalProperties base class and a
//   GenericSignal class deriving from it.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UGenericSignalH
#define UGenericSignalH

#include <vector>
#include "UBCIError.h"

namespace SignalType
{
  typedef enum Type
  {
    int16 = 0,
    float24,
    float32,
    int32,

    numTypes,
    defaultType = float32

  } Type;
  const char* Name( Type );
  bool        ConversionSafe( Type from, Type to );
};

// A class that holds a signal's properties.
class SignalProperties
{
  public:
    SignalProperties()
    : mChannels( 0 ), mElements( 0 ), mType( SignalType::defaultType )                {}

    SignalProperties(
      size_t inChannels,
      size_t inElements,
      SignalType::Type inType = SignalType::defaultType )
    : mChannels( inChannels ), mElements( inElements ), mType( inType )       {}

    virtual ~SignalProperties() {}

    // Read access
    size_t           Channels() const   { return mChannels; }
    size_t           Elements() const   { return mElements; }
    SignalType::Type Type() const       { return mType; }
    // Write access
    size_t& Channels()                  { return mChannels; }
    size_t& Elements()                  { return mElements; }
    SignalType::Type& Type()            { return mType; }

    bool IsEmpty() const                { return Channels() == 0 || Elements() == 0; }
    bool operator==( const SignalProperties& sp ) const
                                        { return Type() == sp.Type() && Elements() == sp.Elements(); }
    bool operator!=( const SignalProperties& sp ) const
                                        { return !( *this == sp ); }
    // These operators tell whether a signal would "fit" into another one.
    // Note that this is essentially a set inclusion relation,
    // so a < b and b >= a may both be false (but not both true) at the same
    // time.
    bool operator>( const SignalProperties& sp ) const
                                        { return ( *this >= sp ) && ( *this != sp ); }
    bool operator<( const SignalProperties& sp ) const
                                        { return ( *this <= sp ) && ( *this != sp ); }
    bool operator>=( const SignalProperties& sp ) const
                                        { return sp <= *this; } 
    bool operator<=( const SignalProperties& ) const;

    // Stream i/o
    void WriteToStream( std::ostream& ) const;
    void ReadFromStream( std::istream& );
    std::ostream& WriteBinary( std::ostream& ) const;
    std::istream& ReadBinary( std::istream& );

#if 1
    // Legacy names for accessors.
    size_t MaxElements() const                          { return Elements(); }
    size_t GetNumElements( size_t /*inChannel*/ ) const { return Elements(); }
#endif

  private:
    size_t           mChannels,
                     mElements;
    SignalType::Type mType;
};

class GenericSignal
{
  public:
    typedef double value_type;

    GenericSignal();
    GenericSignal(
      size_t inChannels,
      size_t inMaxElements,
      SignalType::Type inType = SignalType::defaultType );
    explicit GenericSignal( const SignalProperties& );

    void SetProperties( const SignalProperties& );
    const SignalProperties& GetProperties() const { return mProperties; }

    // Read access
    size_t           Channels() const   { return mProperties.Channels(); }
    size_t           Elements() const   { return mProperties.Elements(); }
    SignalType::Type Type() const       { return mProperties.Type(); }

    // Accessors
    const value_type& GetValue( size_t inChannel, size_t inElement ) const;
    void SetValue( size_t inChannel, size_t inElement, value_type inValue );
    // Bracket read access
    const value_type& operator() ( size_t inChannel, size_t inElement ) const;
    // Bracket write access
    value_type& operator() ( size_t inChannel, size_t inElement );

    // Stream i/o
    void WriteToStream( std::ostream& ) const;
    void ReadFromStream( std::istream& );
    std::ostream& WriteBinary( std::ostream& ) const;
    std::istream& ReadBinary( std::istream& );

#if 1
    // Legacy names for accessors.
    size_t MaxElements() const                          { return Elements(); }
    size_t GetNumElements( size_t /*inChannel*/ ) const { return Elements(); }
#endif

  private:
    template<typename T> static void PutLittleEndian( std::ostream&, const T& );
    template<typename T> static void GetLittleEndian( std::istream&, T& );

  private:
    SignalProperties                      mProperties;
    std::vector<std::vector<value_type> > mValues;
};

inline std::ostream& operator<<( class std::ostream& os, const SignalProperties& s )
{
  s.WriteToStream( os );
  return os;
}

inline std::istream& operator>>( class std::istream& is, SignalProperties& s )
{
  s.ReadFromStream( is );
  return is;
}

inline std::ostream& operator<<( class std::ostream& os, const GenericSignal& s )
{
  s.WriteToStream( os );
  return os;
}

inline std::istream& operator>>( class std::istream& is, GenericSignal& s )
{
  s.ReadFromStream( is );
  return is;
}

#endif // UGenericSignalH

