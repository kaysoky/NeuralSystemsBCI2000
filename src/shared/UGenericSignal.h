////////////////////////////////////////////////////////////////////////////////
//
// File: UGenericSignal.h
//
// Description: This file declares a SignalProperties base class and a BasicSignal
//   class template deriving from it with the signal's numerical type as the
//   template argument.
//   Two classes, GenericSignal and GenericIntSignal, are derived from a float
//   and int instantiation of this template. With a compatibility flag defined
//   (SIGNAL_BACK_COMPAT) old code should compile with minimal changes.
//
//   For the future, the following name transitions might be considered:
//     BasicSignal --> GenericSignal
//     GenericSignal --> FloatSignal
//     GenericIntSignal --> IntSignal
//   as the latter two don't have anything generic about them any more.
//
// Changes: June 28, 2002, juergen.mellinger@uni-tuebingen.de
//          - Rewrote classes from scratch but kept old class interface.
//          Mar 28, 2003, juergen.mellinger@uni-tuebingen.de
//          - Added depth member to SignalProperties to unify GenericSignal
//            and GenericIntSignal into one signal type.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UGenericSignalH
#define UGenericSignalH

#include <vector>
#include "UBCIError.h"

// A properties base class all signals are derived from.
class SignalProperties
{
  public:
    SignalProperties()
    : elements( 0, 0 ), maxElements( 0 ), depth( 0 ) {}
    SignalProperties( size_t inChannels, size_t inMaxElements, size_t inDepth = 0 )
    : elements( inChannels, inMaxElements ), maxElements( inMaxElements ), depth( inDepth ) {}
    virtual ~SignalProperties() {}

    size_t Channels() const { return elements.size(); }
    size_t MaxElements() const { return maxElements; }
    virtual bool SetNumElements( size_t inChannel, size_t inNumElements );
    size_t GetNumElements( size_t inChannel ) const { return elements.at( inChannel ); }
    size_t GetDepth() const { return depth; }
    virtual void SetDepth( size_t inDepth ) { depth = inDepth; }
#ifdef SIGNAL_BACK_COMPAT
    bool SetElements( size_t inCh, size_t inN ) { return SetNumElements( inCh, inN ); }
    size_t GetElements( size_t inCh ) const { return GetNumElements( inCh ); }
#endif // SIGNAL_BACK_COMPAT
    bool operator==( const SignalProperties& sp ) const { return depth == sp.depth && elements == sp.elements; }
    bool operator!=( const SignalProperties& sp ) const { return !( *this == sp ); }
    // These operators tell whether a signal would "fit" into another one.
    // Note that this is essentially a set inclusion relation,
    // so a < b and b >= a may both be false (but not both true) at the same
    // time.
    bool operator>( const SignalProperties& sp ) const { return ( *this >= sp ) && ( *this != sp ); }
    bool operator<( const SignalProperties& sp ) const { return ( *this <= sp ) && ( *this != sp ); }
    bool operator>=( const SignalProperties& ) const;
    bool operator<=( const SignalProperties& ) const;

    // Stream i/o
    void WriteToStream( std::ostream& ) const;
    void ReadFromStream( std::istream& );
    std::ostream& WriteBinary( std::ostream& ) const;
    std::istream& ReadBinary( std::istream& );

  protected:
    std::vector< size_t > elements;
    size_t maxElements,
           depth;
};

// Template declaration of signals.
template< class T > class BasicSignal : public SignalProperties
{
  public:
    typedef T value_type;

    BasicSignal();
    BasicSignal( size_t inChannels, size_t inMaxElements );
    BasicSignal( const SignalProperties& );

    void SetProperties( const SignalProperties& );

    // Overridden from SignalProperties
    virtual bool SetNumElements( size_t inChannel, size_t inElements );

    // Accessors
    const T& GetValue( size_t inChannel, size_t inElement ) const;
    void SetValue( size_t inChannel, size_t inElement, T inValue );
    const std::vector< T >& GetChannel( size_t inChannel ) const;
    void SetChannel( const T* inChannelData, size_t inChannel );

    // Read access
    const T& operator() ( size_t inChannel, size_t inElement ) const;
    // Write access
    T& operator() ( size_t inChannel, size_t inElement );

    // Stream i/o
    void WriteToStream( std::ostream& ) const;
    void ReadFromStream( std::istream& );
    std::ostream& WriteBinary( std::ostream& ) const;
    std::istream& ReadBinary( std::istream& );

#ifndef SIGNAL_BACK_COMPAT
  // For new code, use "SomeSignal( i, j )" resp.
  // "( *SomeSignalPointer )( i, j )" as declared above instead of accessing
  // the Value member directly.
  protected:
#endif // SIGNAL_BACK_COMPAT
    std::vector< std::vector< T > > Value;
};

// Template definitions for the signal members.
template< class T >
BasicSignal< T >::BasicSignal()
: SignalProperties( 0, 0, sizeof( T ) )
{
  SetProperties( *this );
}

template< class T >
BasicSignal< T >::BasicSignal( size_t inChannels, size_t inMaxElements )
: SignalProperties( inChannels, inMaxElements, sizeof( T ) )
{
  SetProperties( *this );
}

template< class T >
BasicSignal< T >::BasicSignal( const SignalProperties& inProperties )
: SignalProperties( inProperties )
{
  SetProperties( *this );
}

template< class T >
bool
BasicSignal< T >::SetNumElements( size_t inChannel, size_t inElements )
{
  bool result = SignalProperties::SetNumElements( inChannel, inElements );
  SetProperties( *this );
  return result;
}

template< class T >
const T&
BasicSignal< T >::GetValue( size_t inChannel, size_t inElement ) const
{
#ifdef SIGNAL_BACK_COMPAT
  static T nullvalue = ( T )0;
  if( ( inChannel >= Value.size() ) || ( inElement >= Value[ inChannel ].size() ) )
    return nullvalue;
#endif // SIGNAL_BACK_COMPAT
  return Value.at( inChannel ).at( inElement );
}

template< class T >
void
BasicSignal< T >::SetValue( size_t inChannel, size_t inElement, T inValue )
{
#ifdef SIGNAL_BACK_COMPAT
  if( ( inChannel >= Value.size() ) || ( inElement >= Value[ inChannel ].size() ) )
    return;
#endif // SIGNAL_BACK_COMPAT
  Value.at( inChannel ).at( inElement ) = inValue;
}

template< class T >
void
BasicSignal< T >::SetChannel( const T* inChannelData, size_t inChannel )
{
  for( size_t i = 0; i < GetChannel( inChannel ).size(); ++i )
    ( *this )( inChannel, i ) = inChannelData[ i ];
}

template< class T >
const std::vector< T >&
BasicSignal< T >::GetChannel( size_t inChannel ) const
{
#ifdef _DEBUG
  return Value.at( inChannel );
#else
  return Value[ inChannel ];
#endif
}

template< class T >
const T&
BasicSignal< T >::operator() ( size_t inChannel, size_t inElement ) const
{
#ifdef _DEBUG
  return Value.at( inChannel ).at( inElement );
#else
  return Value[ inChannel ][ inElement ];
#endif
}

template<class T>
T&
BasicSignal< T >::operator() ( size_t inChannel, size_t inElement )
{
#ifdef _DEBUG
  return Value.at( inChannel ).at( inElement );
#else
  return Value[ inChannel ][ inElement ];
#endif
}

template<class T>
void
BasicSignal< T >::SetProperties( const SignalProperties& inSp )
{
#ifdef _DEBUG
  if( inSp.GetDepth() > sizeof( T ) )
    throw DEBUGINFO "Signal depth greater than allowed for by data type.";
#endif
  Value.resize( inSp.Channels() );
  for( size_t i = 0; i != Value.size(); ++i )
    Value[ i ].resize( inSp.GetNumElements( i ), T( 0 ) );
  *static_cast< SignalProperties* >( this ) = inSp;
}

template<class T>
std::ostream&
BasicSignal< T >::WriteBinary( std::ostream& os ) const
{
  SignalProperties::WriteBinary( os );
  for( size_t j = 0; j < MaxElements(); ++j )
    for( size_t i = 0; i < Value.size(); ++i )
    {
      if( j >= Value[ i ].size() )
      {
        static T null = T( 0 );
        os.write( ( const char* )&null, GetDepth() );
      }
      else
        os.write( ( const char* )&Value[ i ][ j ], GetDepth() );
    }
  return os;
}

template<class T>
std::istream&
BasicSignal< T >::ReadBinary( std::istream& is )
{
  SignalProperties::ReadBinary( is );
  SetProperties( *this );
  for( size_t j = 0; j < MaxElements(); ++j )
    for( size_t i = 0; i < Value.size(); ++i )
    {
      if( j < Value[ i ].size() )
        os.read( ( char* )&Value[ i ][ j ], GetDepth() );
    }
  return is;
}

// Commonly used template instantiations.
// A simple typedef breaks on forward declarations in other headers.
// If anyone knows how to avoid the following clumsiness, please tell.
#if 0
typedef BasicSignal< short > GenericIntSignal;
typedef BasicSignal< float > GenericSignal;
#else
class GenericIntSignal : public BasicSignal< short >
{
  public:
    GenericIntSignal() {}
    GenericIntSignal( const SignalProperties& sp )
    : BasicSignal< short >( sp ) {}
    GenericIntSignal( unsigned short NewChannels, int NewMaxElements )
    : BasicSignal< short >( NewChannels, NewMaxElements ) {}
};

class GenericSignal : public BasicSignal< float >
{
  public:
    GenericSignal() {}
    GenericSignal( const SignalProperties& sp )
    : BasicSignal< float >( sp ) {}
    GenericSignal( unsigned short NewChannels, int NewMaxElements )
    : BasicSignal< float >( NewChannels, NewMaxElements ) {}
    GenericSignal( const GenericIntSignal& intsig ) { *this = intsig; }
    GenericSignal( const GenericIntSignal* intsig ) { *this = *intsig; }
    const GenericSignal& operator=( const GenericIntSignal& );
    void  SetChannel(const short *source, size_t channel);

    void WriteToStream( std::ostream& ) const;
    void ReadFromStream( std::istream& );
    std::ostream& WriteBinary( std::ostream& ) const;
    std::istream& ReadBinary( std::istream& );
};
#endif

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


template<class T>
inline std::ostream& operator<<( class std::ostream& os, const BasicSignal<T>& s )
{
  s.WriteToStream( os );
  return os;
}

template<class T>
inline std::istream& operator>>( class std::istream& is, BasicSignal<T>& s )
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

