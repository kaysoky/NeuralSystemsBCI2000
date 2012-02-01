////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for system states (event markers).
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
#ifndef STATE_H
#define STATE_H

#include <iostream>
#include <cstring>

class StateVector;
class StateList;
class CoreModule;

class State
{
  friend class StateVector; // calls SetLocation(), GetValue(), Commit()
  friend class StateList;   // calls GetValue()
  friend class CoreModule;  // calls GetValue()

 public:
  typedef unsigned long ValueType;

  enum
  {
    StateKind, // element of system state
    EventKind, // event recording

    numKinds
  };

 public:
  State();
  ~State() {}

  bool operator==( const State& ) const;
  bool operator!=( const State& s ) const
                 { return !operator==( s ); }

  const std::string& Name() const
      { return mName; }
  int Kind() const
      { return mKind; }
  State& SetKind( int k )
      { mKind = k; return *this; }
  int Location() const
      { return static_cast<int>( mLocation ); }
  int Length() const
      { return static_cast<int>( mLength ); }

  State& AssignValue( const State& s )
      { return SetValue( s.Value() ); }
  State& SetValue( ValueType );

  std::ostream& WriteToStream( std::ostream& ) const;
  std::istream& ReadFromStream( std::istream& );
  std::ostream& WriteBinary( std::ostream& ) const;
  std::istream& ReadBinary( std::istream& );

  class NameCmp
  {
   public:
    bool operator()( const std::string&, const std::string& ) const;
  };

 private:
  ValueType Value() const
            { return mValue; }
  State&    SetLocation( size_t location )
            { mLocation = location; return *this; }
  State&    SetByteLocation( size_t location )
            { SetLocation( location * 8 + BitLocation() ); return *this; }
  State&    SetBitLocation( size_t location )
            { SetLocation( ByteLocation() * 8 + location ); return *this; }

  size_t    ByteLocation() const
            { return mLocation / 8; }
  size_t    BitLocation() const
            { return mLocation % 8; }

  void      Commit( StateVector* );

 private:
  std::string mName;
  ValueType   mValue;
  size_t      mLocation,
              mLength;
  int         mKind;
  bool        mModified;
};

inline
std::ostream& operator<<( std::ostream& os, const State& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, State& s )
{ return s.ReadFromStream( is ); }

#endif // STATE_H

