////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for a list of BCI2000 states (event markers).
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef STATE_LIST_H
#define STATE_LIST_H

#include <vector>
#include <map>
#include <string>
#include "State.h"
#include "StateVectorSample.h"

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
       { return static_cast<int>( size() ); }
  bool Empty() const
       { return empty(); }
  void Clear();

  int  BitLength() const;
  int  ByteLength() const
       { return BitLength() / 8 + 1; }

  bool Exists( const std::string& name ) const
       { return mIndex.find( name ) != mIndex.end(); }
  int  Index( const std::string& name ) const
       { return Exists( name ) ? mIndex.find( name )->second : Size(); };
  void Add( const State& s )
       { ( *this )[ s.Name() ] = s; }
  bool Add( const std::string& stateDefinition );
  void Delete( const std::string& name );
  void AssignPositions();
  StateVectorSample GetMask( int kind ) const;

  std::ostream& WriteToStream( std::ostream& ) const;
  std::istream& ReadFromStream( std::istream& );
  std::ostream& WriteBinary( std::ostream& ) const;
  std::istream& ReadBinary( std::istream& );

 private:
  void RebuildIndex();

  typedef std::map<std::string, int, State::NameCmp> StateIndex;
  StateIndex mIndex;

  State mDefaultState;
};


inline
std::ostream& operator<<( std::ostream& os, const StateList& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, StateList& s )
{ return s.ReadFromStream( is ); }

#endif // STATE_LIST_H


