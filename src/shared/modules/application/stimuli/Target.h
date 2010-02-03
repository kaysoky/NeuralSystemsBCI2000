////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A virtual base class defining a calling interface for "targets".
//   A target is anything that can be selected by the user.
//   Descendants of Target implement actions in their OnSelect event handler.
//   A speller target, for example, will enter its entry text in its OnSelect
//   method.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TARGET_H
#define TARGET_H

#include <set>

class Target
{
 public:
  Target()
    : mTag( 0 )
    {}
  virtual ~Target()
    {}
  // Properties
  Target& SetTag( int inTag )
    { mTag = inTag; return *this; }
  int Tag() const
    { return mTag; }
  // Event calling
  Target& Select()
    { OnSelect(); return *this; }

 protected:
  virtual void OnSelect()
    {}

 private:
  int mTag;
};

class SetOfTargets : public std::set<Target*>
{
 public:
  SetOfTargets()
    {}
  virtual ~SetOfTargets()
    {}
  // Householding
  SetOfTargets& Add( Target* t )
    { insert( t ); return *this; }
  SetOfTargets& Remove( Target* t )
    { erase( t ); return *this; }
  SetOfTargets& Clear()
    { clear(); return *this; }
  SetOfTargets& DeleteObjects()
    { for( iterator i = begin(); i != end(); ++i ) delete *i; clear(); return *this; }

  bool Contains( Target* t ) const
    { return find( t ) != end(); }
  bool Intersects( const SetOfTargets& s ) const
    {
      for( const_iterator i = begin(); i != end(); ++i )
        if( s.Contains( *i ) ) return true;
      return false;
    }

  // Events
  void Select()
    { for( iterator i = begin(); i != end(); ++i ) ( *i )->Select(); }
};


#endif // TARGET_H

