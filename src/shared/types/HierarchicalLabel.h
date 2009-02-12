////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that handles hierarchical labels, such as a parameter's
//   section label, or a signal's channel labels.
//   Hierarchy levels are separated by a single ':' character.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef HIERARCHICAL_LABEL_H
#define HIERARCHICAL_LABEL_H

#include <iostream>
#include <vector>
#include "EncodedString.h"

class HierarchicalLabel : public std::vector<EncodedString>
{
  public:
    static const char cLevelDelimiter = ':';

    HierarchicalLabel()
      : std::vector<EncodedString>( 1, "" )
      {}
    bool operator<( const HierarchicalLabel& ) const;
    // Stream I/O.
    std::ostream& WriteToStream( std::ostream& ) const;
    std::istream& ReadFromStream( std::istream& );
};

inline
std::ostream& operator<<( std::ostream& s, const HierarchicalLabel& h )
{ return h.WriteToStream( s ); }

inline
std::istream& operator>>( std::istream& s, HierarchicalLabel& h )
{ return h.ReadFromStream( s ); }

#endif // HIERARCHICAL_LABEL_H
