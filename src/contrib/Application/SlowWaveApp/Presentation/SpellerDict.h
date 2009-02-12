/////////////////////////////////////////////////////////////////////////////
//
// File: SpellerDict.h
//
// Date: Dec 13, 2001
//
// Author: Juergen Mellinger
//
// Description: A class that reads and manages a speller dictionary.
//
// Changes:
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef SPELLERDICTH
#define SPELLERDICTH

#include <vector>
#include <string>
#include "PresErrors.h"

class TSpellerDict : private std::vector< std::string >
{
  public:
    TPresError          ReadFromFile( const char*   inDictFileName );
    const std::string&  Lookup( const std::string&  inWordBegin ) const;
};

#endif // SPELLERDICTH
