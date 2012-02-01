////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A program that produces random but valid parameter
//   files for BCI2000 in AR or P300 configuration.
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
////////////////////////////////////////////////////////////////////
#include "ParamEnv.h"
#include "SourceGenerator.h"
#include "SpatialGenerator.h"
#include "ARGenerator.h"
#include "P3Generator.h"
#include "PrecisionTime.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

int
main( int argc, char* argv[] )
{
  enum { none, AR, P3 } config = none;

  if( argc > 1 )
  {
    if( string( "AR" ) == argv[1] || string( "ar" ) == argv[1] )
      config = AR;
    else if( string( "P3" ) == argv[1] || string( "p3" ) == argv[1] )
      config = P3;
  }

  if( config == none )
  {
    cerr << "Please specify a target configuration for the generator.\n"
         << "Valid target configurations are P3 and AR."
         << endl;
    return -1;
  }

  if( argc > 2 )
  {
    int seed = ::atoi( argv[2] );
    ::srand( seed );
  }
  else
  {
    ::srand( static_cast<unsigned int>( ::time( NULL ) ) + PrecisionTime::Now() );
  }

  ParamEnv params;

  vector<Generator*> generators;
  generators.push_back( new SourceGenerator );
  generators.push_back( new SpatialGenerator );
  switch( config )
  {
    case AR:
      generators.push_back( new ARGenerator );
      break;
    case P3:
      generators.push_back( new P3Generator );
      break;
    case none:
      break;
  }
  for( vector<Generator*>::iterator i = generators.begin(); i != generators.end(); ++i )
  {
    ( *i )->Generate( params );
    delete *i;
  }

  cout << params;

  return 0;
}
