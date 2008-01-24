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
#include "PCHIncludes.h"
#pragma hdrstop

#include "RandomGenerator.h"
#include <cstdlib>
#include <cassert>

using namespace std;

RandomGenerator::NumberType
RandomGenerator::RandMax() const
{
  return NumberFromSeed( ~SeedType( 0 ) );
}

RandomGenerator::NumberType
RandomGenerator::Random()
{ // x_i = ( x_{i-1} * a + c ) mod m
  // The a parameter is chosen to be optimal for m = 2^32 according to:
  // P. L'Ecuyer. A table of linear congruential generators of different sizes
  // and good lattice structure. Mathematics of Computation, 68(225), 1999.
  // Using uint32 for x, mod m is computed implicitly.
  const uint32 a = 2891336453,
               c = 1;
  mSeed *= a;
  mSeed += c;
  return NumberFromSeed( mSeed );
}

RandomGenerator::NumberType
RandomGenerator::NumberFromSeed( RandomGenerator::SeedType inSeed )
{
  return inSeed >> 16; // Ignore lower bits which have bad periodicity.
}

void
RandomGenerator::Preflight() const
{
  OptionalParameter( "RandomSeed" );
}

void
RandomGenerator::Initialize()
{
  SeedType seed = OptionalParameter( "RandomSeed", 0 );
  if( seed == 0 )
    mSeed = ::time( NULL );
  else
    mSeed = seed;
}

void
RandomGenerator::StartRun()
{
  Initialize();
}

