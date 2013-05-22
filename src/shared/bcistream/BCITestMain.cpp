//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A main file to run BCITests contained in libraries.
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
///////////////////////////////////////////////////////////////////////
#pragma hdrstop

#if BCITEST_MAIN
# undef main
# include "BCITest.h"
int main( int, char** )
{
  return bci::Test::RunTests();
}
#elif defined( main ) && ( main == bcitest_main_ )
# include "BCITest.h"
# define ACTUAL_MAIN_ main
# undef main

int ACTUAL_MAIN_( int, char** );

int main( int argc, char** argv )
{
  if( bci::Test::Parse( argc, argv ) )
    return bci::Test::RunTests();
  return ACTUAL_MAIN_( argc, argv );
}
#endif // main
