////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A speller provides an Enter() method, and keeps a collection of
//   speller targets.
//   When a speller target is selected, it calls the speller's Enter() method
//   with the value of its EntryText property.
//   The speller's SuggestTarget() method returns the target best suited to modify
//   its first argument string into its second argument string.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SPELLER_H
#define SPELLER_H

#include "Target.h"
#include <string>
#include <set>

class SpellerTarget;
typedef std::set<SpellerTarget*> SetOfSpellerTargets;

class Speller
{
 public:
  Speller();
  virtual ~Speller();
  // Properties
  Speller& Add( SpellerTarget* );
  Speller& Remove( SpellerTarget* );
  Speller& DeleteObjects();

  // Target suggestion
  virtual SpellerTarget* SuggestTarget( const std::string& from,
                                        const std::string& to ) const;
  // Event triggering
  Speller& Enter( const std::string& );

 protected:
  virtual void OnEnter( const std::string& ) = 0;

 private:
  SetOfSpellerTargets mTargets;
};


class SpellerTarget : public Target
{
 public:
  SpellerTarget( Speller& );
  virtual ~SpellerTarget();
  // Properties
  SpellerTarget& SetEntryText( const std::string& );
  const std::string& EntryText() const;

 protected:
  virtual void OnSelect();

 private:
  Speller&    mSpeller;
  std::string mEntryText;
};

#endif // SPELLER_H

