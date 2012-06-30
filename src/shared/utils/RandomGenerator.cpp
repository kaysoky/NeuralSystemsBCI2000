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
#include "PCHIncludes.h"
#pragma hdrstop

#include "RandomGenerator.h"
#include <cstdlib>

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
  // Using uint32_t for x, mod m is computed implicitly.
  const uint32_t a = 2891336453UL,
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
    mSeed = static_cast<SeedType>( ::time( NULL ) );
  else
    mSeed = seed;
}

void
RandomGenerator::StartRun()
{
  Initialize();
}

