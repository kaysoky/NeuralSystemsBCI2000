////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarland@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates a random number generator.
//   Each class instance maintains its own internal state, ensuring identical
//   number sequences for identical seeds across software elements that use
//   random numbers.
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
#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include "Environment.h"
#include "defines.h"

class RandomGenerator : private EnvironmentExtension
{
 public:
  typedef uint32_t SeedType;
  typedef uint32_t NumberType;

#ifdef TODO
# error Modify ID determination to use a filter´s unique ID once multiple filter instances are possible.
#endif
  template<typename T>
  explicit RandomGenerator( const T* t )
    : mSeed( 0 ),
      mID( bci::ClassName( typeid( T ) ) )
    {}
  explicit RandomGenerator( const std::string& s )
    : mSeed( 0 ),
      mID( s )
    {}
  RandomGenerator();
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
  template<typename Int> Int operator()( Int inN )
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
  std::string mID;
  static int NextUnnamedInstance();
};

#endif // RANDOM_GENERATOR_H
