//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A string class that allows for transparent handling of
//   character codes using the '%' character.
//
//   WriteToStream() will always output white space in encoded
//   hexadecimal form.
//   Additional characters may be listed in the "encodeThese"
//   parameter.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#ifndef ENCODED_STRING_H
#define ENCODED_STRING_H

#include <string>
#include <iostream>

class EncodedString : public std::string
{
 public:
  EncodedString();
  EncodedString( const std::string& );
  EncodedString( const char* );

  // The "encodeThese" parameter lists additional characters that will be written
  // in encoded form.
  std::ostream& WriteToStream( std::ostream&, const std::string& encodeThese = "" ) const;
  std::istream& ReadFromStream( std::istream& );
};

inline
EncodedString::EncodedString()
{
}

inline
EncodedString::EncodedString( const std::string& s )
: std::string( s )
{
}

inline
EncodedString::EncodedString( const char* s )
: std::string( s )
{
}

inline
std::ostream& operator<<( std::ostream& s, const EncodedString& e )
{
  return e.WriteToStream( s );
}

inline
std::istream& operator>>( std::istream& s, EncodedString& e )
{
  return e.ReadFromStream( s );
}

#endif // ENCODED_STRING_H
