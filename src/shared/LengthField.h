////////////////////////////////////////////////////////////////////////////////
//
// File: LengthField.h
//
// Description: A template for length fields that occur in BCI2000 messages,
//   e.g. for the length of the message, or for the number of channels
//   and elements in a data message.
//
//   Some of the fixed-length fields have proven too short, so there is a
//   backward compatible extension introduced: An N-bit fixed-length entry
//   may contain a value of 2^N-1; in that case, a null-terminated ASCII
//   representation of the actual length follows.
//   If the represented value does not exceed 2^N-2, it is always transmitted
//   within in the fixed-length field, and not followed by a string.
//
//   These conventions ensure inter-operability between different versions of
//   the BCI2000 protocol as long as messages and signals have appropriate
//   sizes/dimensions.
//
// Author: Juergen Mellinger
//
// Date:   Jun 18, 2004
//
////////////////////////////////////////////////////////////////////////////////
#ifndef LengthFieldH
#define LengthFieldH

#include <iostream>

template<int NumBytes> class LengthField
{
  public:
    enum
    {
      EscapeValue = ( 1 << ( 8 * NumBytes ) ) - 1, // NumBytes bytes filled with 0xff.
    };
    LengthField() : mValue( 0 ) {}
    LengthField( size_t value ) : mValue( value ) {}

    // Type conversion:
    operator size_t&() { return mValue; }

    // Formatted (ASCII) I/O:
    void WriteToStream( std::ostream& os ) const { os << mValue; }
    void ReadFromStream( std::istream& is ) { is >> mValue; }

    // Binary I/O according to the BCI2000 protocol:
    void WriteBinary( std::ostream& os ) const;
    void ReadBinary( std::istream& is );

  private:
    size_t mValue;
};

template<int NumBytes> std::ostream&
operator<<( std::ostream& os, const LengthField<NumBytes>& l )
{
  l.WriteToStream( os );
  return os;
}

template<int NumBytes> std::istream&
operator<<( std::ostream& is, LengthField<NumBytes>& l )
{
  l.ReadFromStream( is );
  return is;
}

template<int NumBytes> void
LengthField<NumBytes>::WriteBinary( std::ostream& os ) const
{
  size_t value = mValue;
  if( value > EscapeValue )
    value = EscapeValue;
  // Old protocol: Write bytes of mValue in little endian order.
  // Extended protocol: Write the escape value.
  for( int i = 0; i < NumBytes; ++i )
  {
    os.put( value & 0xff );
    value >>= 8;
  }
  if( mValue >= EscapeValue )
  {
    // Extended protocol: Write a null-terminated ASCII representation of the
    // value after the escape value.
    os << mValue;
    os.put( '\0' );
  }
}

template<int NumBytes> void
LengthField<NumBytes>::ReadBinary( std::istream& is )
{
  mValue = 0;
  // Read a little endian value according to the old protocol.
  for( int i = 0; i < NumBytes; ++i )
    mValue |= ( is.get() << ( 8 * i ) );
  // If the value matches the escape value, read a null-terminated
  // ASCII representation.
  if( mValue == EscapeValue )
  {
    is >> mValue;
    if( is.get() != '\0' )
      is.setstate( is.failbit );
  }
}

#endif // LengthFieldH

