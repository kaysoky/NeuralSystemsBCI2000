////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarland@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates a random number generator.
//   Each class instance maintains its own internal state, ensuring identical
//   number sequences for identical seeds across software elements that use
//   random numbers.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include "Environment.h"
#include "defines.h"

class RandomGenerator : private EnvironmentExtension
{
 public:
  typedef uint32 SeedType;
  typedef uint32 NumberType;

  explicit RandomGenerator()
    : mSeed( 0 )
    {}
  virtual ~RandomGenerator()
    {}
  // Properties
  SeedType Seed() const
    { return mSeed; }
  RandomGenerator& SetSeed( SeedType s )
    { mSeed = s; return *this; }

  virtual NumberType RandMax() const;

  // This returns a random integer between 0 and including RandMax().
  virtual NumberType Random();

  // STL functor interface: operator() returns a random integer between 0 and (N-1).
  template<typename Int> operator()( Int inN )
    { return ( Random() * inN ) / ( RandMax() + 1 ); }

  // EnvironmentExtension interface.
  void Publish()
    {}
  void Preflight() const;
  void Initialize();
  void StartRun();
  void Process()
    {}

 private:
  static NumberType NumberFromSeed( SeedType );

  SeedType mSeed;
};

#endif // RANDOM_GENERATOR_H
