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
#include "FileUtils.h"
#include "BCIError.h"
#include "BCIAssert.h"
#include <cstdlib>

using namespace std;

// For backward compatibility, allow for the instance count to serve as an ID
int
RandomGenerator::NextUnnamedInstance()
{
  static int count = 0;
  return ++count;
}

RandomGenerator::RandomGenerator()
: mSeed( 0 )
{
  int count = NextUnnamedInstance();
  mID.resize( 1 );
  if( count >= 256 )
    throw bciexception(
      "Number of unnamed RandomGenerator instances exceeds limit, "
      "use constructor arguments to name RandomGenerators"
    );
  mID[0] = static_cast<string::value_type>( count );
  mID += FileUtils::ApplicationTitle();
}

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
  // Use the ID string to modify the seed in a way that is both unique 
  // and robust against configuration changes such as addition of filters,
  // or change of endianness.
  while( mID.length() % sizeof( SeedType ) )
    mID += " ";
  for( size_t i = 0; i < mID.length() / sizeof( SeedType ); ++i )
  {
    SeedType value = 0;
    for( size_t j = 0; j < sizeof( SeedType ); ++j )
    {
      SeedType c = static_cast<uint8>( mID[i * sizeof( SeedType ) + j] );
      value |= c << ( 8 * j );
    }
    mSeed ^= value;
  }
  bcidbg( 1 ) << "RandomGenerator ID: " << mID << ", Seed: " << hex << mSeed << endl;
}

void
RandomGenerator::StartRun()
{
  Initialize();
}

