//////////////////////////////////////////////////////////////////////
//
// File: EncodedString.cpp
//
// Date: Oct 31, 2005
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A class that allows for convenient automatic type
//         conversions when accessing parameter values.
//
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "EncodedString.h"
#include <sstream>

using namespace std;

static const char escapeChar = '%';

// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of a single
//             string value.
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
EncodedString::ReadFromStream( istream& is )
{
  string newContent;
  if( is >> newContent )
  {
    size_t pos = newContent.find( escapeChar, 0 );
    while( pos != npos )
    {
      newContent.erase( pos, 1 );

      size_t numDigits = 0;
      char curDigit;
      int hexValue = 0;
      while( pos + numDigits < newContent.size() && numDigits < 2
             && ::isxdigit( curDigit = newContent[ pos + numDigits ] ) )
      {
        if( !::isdigit( curDigit ) )
          curDigit = ::toupper( curDigit ) - 'A' + 10;
        else
          curDigit -= '0';
        hexValue = ( hexValue << 4 ) + curDigit;
        ++numDigits;
      }
      newContent.erase( pos, numDigits );
      if( hexValue > 0 )
        newContent.insert( pos, 1, ( char )hexValue );

      pos = newContent.find( escapeChar, pos + 1 );
    }
    *this = newContent;
  }
  return is;
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of a single
//             encoded string value.
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into; list of characters that may not
//             appear in the output.
// Returns:    Stream written into.
// **************************************************************************
ostream&
EncodedString::WriteToStream( ostream& os, const string& forbiddenChars ) const
{
  if( empty() )
    os << escapeChar;
  else
  {
    const string& self = *this;
    ostringstream oss;
    oss << hex;
    for( size_t pos = 0; pos < size(); ++pos )
    {
      if( ::isprint( self[ pos ] )
          && !::isspace( self[ pos ] )
          && forbiddenChars.find( self[ pos ] ) == npos )
      {
        oss << self[ pos ];
        if( self[ pos ] == escapeChar )
          oss << escapeChar;
      }
      else
        oss << escapeChar
            << ( int )( ( self[ pos ] >> 4 ) & 0x0f )
            << ( int )( self[ pos ] & 0x0f );
    }
    os << oss.str();
  }
  return os;
}



