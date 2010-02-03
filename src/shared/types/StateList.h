////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for a list of BCI2000 states (event markers).
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef STATE_LIST_H
#define STATE_LIST_H

#include <vector>
#include <map>
#include <string>
#include "State.h"

typedef std::vector<State> StateContainer;

class StateList : private StateContainer
{
 public:
  bool         operator==( const StateList& ) const;
  bool         operator!=( const StateList& s ) const
               { return !operator==( s ); }

  const State& operator[]( const std::string& ) const;
  State&       operator[]( const std::string& );
  const State& operator[]( size_t idx ) const
               { return at( idx ); }
  State&       operator[]( size_t idx )
               { return at( idx ); }

  int  Size() const
       { return size(); }
  bool Empty() const
       { return empty(); }
  void Clear();

  bool Exists( const std::string& name ) const
       { return mIndex.find( name ) != mIndex.end(); }
  int  Index( const std::string& name ) const
       { return Exists( name ) ? mIndex.find( name )->second : Size(); };
  void Add( const State& s )
       { ( *this )[ s.Name() ] = s; }
  bool Add( const std::string& stateDefinition );
  void Delete( const std::string& name );
  void AssignPositions();

  std::ostream& WriteToStream( std::ostream& ) const;
  std::istream& ReadFromStream( std::istream& );
  std::ostream& WriteBinary( std::ostream& ) const;
  std::istream& ReadBinary( std::istream& );

 private:
  void RebuildIndex();

  typedef std::map<std::string, int, State::NameCmp> StateIndex;
  StateIndex mIndex;
};


inline
std::ostream& operator<<( std::ostream& os, const StateList& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, StateList& s )
{ return s.ReadFromStream( is ); }

#endif // STATE_LIST_H


