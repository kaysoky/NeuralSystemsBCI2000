////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: This helper class defines what we accept as delimiting
//   single-character symbol pairs for index lists and sub-parameters in a
//   parameter line.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BRACKETS_H
#define BRACKETS_H

#include <string>

class Brackets
{
 public:
  static bool IsOpening( char c )
  {
    return ClosingMatch( c ) != '\0';
  }
  static char ClosingMatch( char c )
  {
    size_t pos = BracketPairs().find( c );
    if( pos != std::string::npos && !( pos & 1 ) ) // opening brackets are at even positions
      return BracketPairs()[ pos + 1 ];
    return '\0';
  }
  static const std::string& BracketPairs();
  // A bracket pair that is used for writing.
  static const char OpeningDefault = '{';
  static const char ClosingDefault = '}';
};

#endif // BRACKETS_H

