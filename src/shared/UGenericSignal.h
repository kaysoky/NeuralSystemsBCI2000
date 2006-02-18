////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: UGenericSignal.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// $Log$
// Revision 1.21  2006/02/18 12:07:21  mellinger
// Introduced min() and max() members into SignalType class.
//
// Revision 1.20  2006/02/03 13:40:53  mellinger
// Compatibility with gcc and BCB 2006.
//
// Revision 1.19  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
//
// Description: This file declares a SignalProperties base class and a
//   GenericSignal class deriving from it.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UGenericSignalH
#define UGenericSignalH

#include <vector>
#include <iostream>
#include "UBCIError.h"

class SignalType
{
 public:
  typedef enum Type
  {
    none = -1,
    int16 = 0,
    float24,
    float32,
    int32,

    numTypes,
    defaultType = float32

  } Type;

  SignalType() : mType( none )      {}
  SignalType( Type t ) : mType( t ) {}
  operator SignalType::Type() const { return mType; }

  const char* Name() const;
  size_t      Size() const;
  double      Min()  const;
  double      Max()  const;
  static bool ConversionIsSafe( SignalType from, SignalType to );

  void ReadFromStream( std::istream& );
  void WriteToStream( std::ostream& ) const;
  void ReadBinary( std::istream& );
  void WriteBinary( std::ostream& ) const;

 private:
  Type mType;
};

// A class that holds a signal's properties.
class SignalProperties
{
  public:
    SignalProperties()
    : mChannels( 0 ), mElements( 0 ), mType( SignalType::defaultType )  {}

    SignalProperties(
      size_t inChannels,
      size_t inElements,
      SignalType::Type inType = SignalType::defaultType )
    : mChannels( inChannels ), mElements( inElements ), mType( inType ) {}

    SignalProperties(
      size_t inChannels,
      size_t inElements,
      SignalType inType )
    : mChannels( inChannels ), mElements( inElements ), mType( inType ) {}

    virtual ~SignalProperties() {}

    // Read access
    size_t            Channels() const  { return mChannels; }
    size_t            Elements() const  { return mElements; }
    const SignalType& Type() const      { return mType; }
    // Write access
    size_t& Channels()                  { return mChannels; }
    size_t& Elements()                  { return mElements; }
    SignalType& Type()                  { return mType; }

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
    size_t     mChannels,
               mElements;
    SignalType mType;
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
    GenericSignal(
      size_t inChannels,
      size_t inMaxElements,
      SignalType inType );
    explicit GenericSignal( const SignalProperties& );

    void SetProperties( const SignalProperties& );
    const SignalProperties& GetProperties() const { return mProperties; }

    // Read access
    size_t            Channels() const   { return mProperties.Channels(); }
    size_t            Elements() const   { return mProperties.Elements(); }
    const SignalType& Type() const       { return mProperties.Type(); }

#if 1
    // Legacy names for accessors.
    size_t MaxElements() const                          { return Elements(); }
    size_t GetNumElements( size_t /*inChannel*/ ) const { return Elements(); }
#endif

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
    void WriteValueBinary( std::ostream&, size_t inChannel, size_t inElement ) const;
    void ReadValueBinary( std::istream&, size_t inChannel, size_t inElement );
    std::ostream& WriteBinary( std::ostream& ) const;
    std::istream& ReadBinary( std::istream& );

    template<SignalType::Type>
      void PutValueBinary( std::ostream&, size_t inChannel, size_t inElement ) const;
    template<SignalType::Type>
      void GetValueBinary( std::istream&, size_t inChannel, size_t inElement );

  private:
    template<typename T> static void PutLittleEndian( std::ostream&, const T& );
    template<typename T> static void GetLittleEndian( std::istream&, T& );

  private:
    SignalProperties                      mProperties;
    std::vector<std::vector<value_type> > mValues;
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


inline std::ostream& operator<<( std::ostream& os, const SignalType& s )
{
  s.WriteToStream( os );
  return os;
}

inline std::istream& operator>>( std::istream& is, SignalType& s )
{
  s.ReadFromStream( is );
  return is;
}

inline std::ostream& operator<<( std::ostream& os, const SignalProperties& s )
{
  s.WriteToStream( os );
  return os;
}

inline std::istream& operator>>( std::istream& is, SignalProperties& s )
{
  s.ReadFromStream( is );
  return is;
}

inline std::ostream& operator<<( std::ostream& os, const GenericSignal& s )
{
  s.WriteToStream( os );
  return os;
}

inline std::istream& operator>>( std::istream& is, GenericSignal& s )
{
  s.ReadFromStream( is );
  return is;
}

#endif // UGenericSignalH

