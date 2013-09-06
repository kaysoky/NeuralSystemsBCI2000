////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates a Linear Congruential Pseudo Random
//   number generator.
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
#include "LCRandomGenerator.h"

#if _WIN32
# include <Windows.h>
#else
# include <sys/time.h>
#endif

#include <ctime>

using namespace std;

LCRandomGenerator::SeedType
LCRandomGenerator::DefaultSeed()
{
#if _WIN32
  LARGE_INTEGER count;
  if( ::QueryPerformanceCounter( &count ) )
    return static_cast<SeedType>( count.QuadPart );
#else
  timeval t;
  if( !::gettimeofday( &t, 0 ) )
    return static_cast<SeedType>( t.tv_usec );
#endif
  return static_cast<SeedType>( ::time( 0 ) );
}

LCRandomGenerator::NumberType
LCRandomGenerator::RandMax() const
{
  return NumberFromSeed( ~SeedType( 0 ) );
}

LCRandomGenerator::NumberType
LCRandomGenerator::Random()
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

LCRandomGenerator::NumberType
LCRandomGenerator::NumberFromSeed( LCRandomGenerator::SeedType inSeed )
{
  return inSeed >> 16; // Ignore lower bits which have bad periodicity.
}

string
LCRandomGenerator::RandomName( size_t inLength )
{
  string s;
  if( inLength )
    s += RandomCharacter( &::isalpha );
  while( s.length() < inLength )
    s += RandomCharacter( &::isalnum );
  return s;
}

char
LCRandomGenerator::RandomCharacter( int (*inClass)( int ) )
{
  char c = 0;
  do
    c = Random() & 0x7f;
  while( !inClass( c ) );
  return c;
}
