////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for system states (event markers).
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef STATE_H
#define STATE_H

#include <iostream>

class StateVector;

class State
{
  friend class StateVector; // calls SetLocation(), GetValue(), Commit()
  friend class StateList;   // calls GetValue()
  friend class CoreModule;  // calls GetValue()

 public:
  typedef unsigned long ValueType;

 public:
  State();
  ~State() {}

  bool operator==( const State& ) const;
  bool operator!=( const State& s ) const
                 { return !operator==( s ); }

  const std::string& Name() const
      { return mName; }
  int Location() const
      { return mLocation; }
  int Length() const
      { return mLength; }

  State& SetValue( ValueType );

  std::ostream& WriteToStream( std::ostream& ) const;
  std::istream& ReadFromStream( std::istream& );
  std::ostream& WriteBinary( std::ostream& ) const;
  std::istream& ReadBinary( std::istream& );

  class NameCmp
  {
   public:
    bool operator()( const std::string& a, const std::string& b ) const
    { return ::stricmp( a.c_str(), b.c_str() ) < 0; }
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

#ifdef TODO
# error Try to do without the Commit() function.
#endif
  void      Commit( StateVector* );

 private:
  std::string mName;
  ValueType   mValue;
  size_t      mLocation,
              mLength;
  bool        mModified;
};

inline
std::ostream& operator<<( std::ostream& os, const State& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, State& s )
{ return s.ReadFromStream( is ); }

#endif // STATE_H

