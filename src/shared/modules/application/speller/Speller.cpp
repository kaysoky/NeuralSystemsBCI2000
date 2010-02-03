////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A speller provides an Enter() method, and keeps a collection of
//   speller targets.
//   When a speller target is selected, it calls the speller's Enter() method
//   with the value of its EntryText property.
//   The speller's NextTarget() method returns the target best suited to modify
//   its SpelledText() property into its GoalText() property.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Speller.h"
#include <map>

using namespace std;

// Speller members
Speller::Speller()
{
}

Speller::~Speller()
{
}

// Properties
Speller&
Speller::Add( SpellerTarget* s )
{
  mTargets.insert( s );
  return *this;
}

Speller&
Speller::Remove( SpellerTarget* s )
{
  mTargets.erase( s );
  return *this;
}

Speller&
Speller::DeleteObjects()
{
  for( SetOfSpellerTargets::iterator i = mTargets.begin(); i != mTargets.end(); ++i )
    delete *i;
  mTargets.clear();
  return *this;
}

// Target suggestion
SpellerTarget*
Speller::SuggestTarget( const string& inFrom, const string& inTo ) const
{
  // This default method uses the number of characters to delete, plus the number
  // of characters to enter, as a distance metric between strings to determine
  // the optimum target.
  struct
  {
    int operator()( const string& from, const string& to )
    {
      int result = from.length() + to.length();
      string::const_iterator p1 = from.begin(),
                             p2 = to.begin();
      while( p1 != from.end() && p2 != to.end() && *p1++ == *p2++ )
        --result;
      return result;
    };
  } Distance;

  map<int, SpellerTarget*> sortedTargets;
  for( SetOfSpellerTargets::const_iterator i = mTargets.begin(); i != mTargets.end(); ++i )
    sortedTargets[ Distance( inFrom + ( *i )->EntryText(), inTo ) ] = *i;
  return sortedTargets.empty() ? NULL : sortedTargets.begin()->second;
}

// Event triggering
Speller&
Speller::Enter( const std::string& s )
{
  OnEnter( s );
  return *this;
}

// SpellerTarget members
SpellerTarget::SpellerTarget( Speller& s )
: mSpeller( s )
{
  mSpeller.Add( this );
}

SpellerTarget::~SpellerTarget()
{
}

// Properties
SpellerTarget&
SpellerTarget::SetEntryText( const std::string& s )
{
  mEntryText = s;
  return *this;
}

const std::string&
SpellerTarget::EntryText() const
{
  return mEntryText;
}

void SpellerTarget::OnSelect()
{
  mSpeller.Enter( mEntryText );
}

