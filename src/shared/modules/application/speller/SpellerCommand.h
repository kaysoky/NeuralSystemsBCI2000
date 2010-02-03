////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents a speller command (given in <>) which
//   can read and write itself from/to a stream.
//   For plain strings without command codes, the Code property is empty, and
//   the Value property contains the string.
//   Grammar is
//    plain_string | < code value >
//   with optional white space within brackets.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SPELLER_COMMAND_H
#define SPELLER_COMMAND_H

#include <iostream>
#include <string>

class SpellerCommand
{
 public:
  static const char cOpenChar = '<',
                    cCloseChar = '>';

  SpellerCommand()
    {}
  SpellerCommand( const std::string& );
  virtual ~SpellerCommand()
    {}
  // Properties
  SpellerCommand& SetCode( std::string& s )
    { mCode = s; return *this; }
  const std::string& Code() const
    { return mCode; }
  SpellerCommand& SetValue( std::string& s )
    { mValue = s; return *this; }
  const std::string& Value() const
    { return mValue; }
  // Formatted IO
  std::istream& ReadFromStream( std::istream& );
  std::ostream& WriteToStream( std::ostream& ) const;

 private:
  std::string mCode,
              mValue;
};

inline
std::istream& operator>>( std::istream& is, SpellerCommand& s )
{ return s.ReadFromStream( is ); }
inline
std::ostream& operator<<( std::ostream& os, const SpellerCommand& s )
{ return s.WriteToStream( os ); }


#endif // SPELLER_COMMAND_H