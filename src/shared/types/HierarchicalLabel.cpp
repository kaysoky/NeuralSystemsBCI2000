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
#include "PCHIncludes.h"
#pragma hdrstop

#include "HierarchicalLabel.h"
#include <sstream>

using namespace std;

bool
HierarchicalLabel::operator<( const HierarchicalLabel& s ) const
{
  for( size_t i = 0; i < size() && i < s.size(); ++i )
    if( ( *this )[ i ] < s[ i ] )
      return true;
    else if( s[ i ] < ( *this )[ i ] )
      return false;
  return size() < s.size();
}

istream&
HierarchicalLabel::ReadFromStream( istream& is )
{
  clear();
  EncodedString value;
  is >> value;
  istringstream iss( value );
  while( getline( iss, value, cLevelDelimiter ) )
    push_back( value );
  return is;
}

ostream&
HierarchicalLabel::WriteToStream( ostream& os ) const
{
  if( empty() )
    os << EncodedString( "" );
  else
    os << ( *this )[ 0 ];
  for( size_t i = 1; i < size(); ++i )
    os << cLevelDelimiter << ( *this )[ i ];
  return os;
}
