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
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UGenericSignalH
#define UGenericSignalH

#include <vector>

// A properties base class all signals are derived from.
class SignalProperties
{
  public:
    SignalProperties( size_t inChannels, size_t inMaxElements )
    : elements( inChannels, 0 ), maxElements( inMaxElements ) {}
    virtual ~SignalProperties() {}

    virtual size_t Channels() const { return elements.size(); }
    virtual size_t MaxElements() const { return maxElements; }
    virtual bool SetNumElements( size_t inChannel, size_t inNumElements );
    virtual size_t GetNumElements( size_t inChannel ) const { return elements.at( inChannel ); }
#ifdef SIGNAL_BACK_COMPAT
    bool SetElements( size_t inCh, size_t inN ) { return SetNumElements( inCh, inN ); }
    size_t GetElements( size_t inCh ) const { return GetNumElements( inCh ); }
#endif // SIGNAL_BACK_COMPAT
    bool operator==( const SignalProperties& sp ) const { return elements == sp.elements; }
    bool operator!=( const SignalProperties& sp ) const { return !( *this == sp ); }
    // These operators tell whether a signal would "fit" into another one.
    // Note that this is essentially a set inclusion relation,
    // so a < b and b >= a may both be false (but not both true) at the same
    // time.
    bool operator>( const SignalProperties& sp ) const { return ( *this >= sp ) && ( *this != sp ); }
    bool operator<( const SignalProperties& sp ) const { return ( *this <= sp ) && ( *this != sp ); }
    bool operator>=( const SignalProperties& ) const;
    bool operator<=( const SignalProperties& ) const;

  protected:
    std::vector< size_t > elements;
    size_t maxElements;
};

// Template declaration of signals.
template< class T > class BasicSignal : public SignalProperties
{
  public:
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
: SignalProperties( 0, 0 )
{
  SetProperties( *this );
}

template< class T >
BasicSignal< T >::BasicSignal( size_t inChannels, size_t inMaxElements )
: SignalProperties( inChannels, inMaxElements )
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
  return Value.at( inChannel ).at( inElement );
}

template< class T >
void
BasicSignal< T >::SetValue( size_t inChannel, size_t inElement, T inValue )
{
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

template <class T>
T&
BasicSignal< T >::operator() ( size_t inChannel, size_t inElement )
{
#ifdef _DEBUG
  return Value.at( inChannel ).at( inElement );
#else
  return Value[ inChannel ][ inElement ];
#endif
}

template <class T>
void
BasicSignal< T >::SetProperties( const SignalProperties& inSp )
{
  Value.resize( inSp.Channels() );
  for( size_t i = 0; i != Value.size(); ++i )
    Value[ i ].resize( inSp.GetNumElements( i ) );
  static_cast< SignalProperties >( *this ) = inSp;
}

// Commonly used template instantiations.

// A simple typedef breaks on forward declarations in other headers.
// If anyone knows how to avoid the following clumsiness, please tell.
class GenericIntSignal : public BasicSignal< short >
{
  public:
    GenericIntSignal( unsigned short NewChannels, int NewMaxElements )
    : BasicSignal< short >( NewChannels, NewMaxElements ) {}
};

class GenericSignal : public BasicSignal< float >
{
  public:
    GenericSignal( unsigned short NewChannels, int NewMaxElements )
    : BasicSignal< float >( NewChannels, NewMaxElements ) {}
    GenericSignal( const GenericIntSignal& intsig ) { *this = intsig; }
    GenericSignal( const GenericIntSignal* intsig ) { *this = *intsig; }
    const GenericSignal& operator=( const GenericIntSignal& );
    void  SetChannel(const short *source, size_t channel);
};

#endif // UGenericSignalH

