//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class and macros that allow to embed testing code
//   within source code files.
//
//   #include "BCITest.h"
//
//   bcitest( <name> )
//   {
//     // testing code
//     ...
//     if( failed )
//       bcifail << message;
//     // or
//     bcifail_if( condition, message << data );
//   }
//  
//   In CMake, add a test by calling
//   BCI2000_ADD_UNIT_TEST( source1.cpp source2.cpp ... )
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
#ifndef BCI_TEST_H
#define BCI_TEST_H

#include "BCIRegistry.h"
#include "BCIStream.h"
#include <vector>
#include <sstream>

#if DISABLE_BCITEST

# define bcitest( name ) \
static struct name##_ { name##_() {} private: void OnRun(); } name; \
void name##_::OnRun()

# define bcifail BCIStream::Nullstream()
# define bcifail_if( cond, msg )

#else // DISABLE_BCITEST

# define bcitest( name ) \
static struct name##_ : bci::Test { name##_() : bci::Test( #name ) {} private: void OnRun(); } name; \
RegisterTest_( name ); void name##_::OnRun()

# define bcifail FailStream( __FILE__ , __LINE__ )
# define bcifail_if( cond, msg ) if( cond ) bcifail << #cond << ", " << msg << std::endl;

#endif // DISABLE_BCITEST

namespace bci {

class Test
{
 public:
  typedef void(*FailHandler)( const std::string& );
  
  Test( const std::string& name );
  bool Run( FailHandler = NULL );

  struct RunTests
  {
    RunTests( FailHandler = 0 );
    operator int() const { return failures; }
    private: int failures;
  };
  static int Parse( int argc, char** argv, bool exitIfFound = true );

 protected:
  std::ostream& FailStream( const char*, int );
  virtual void OnRun() = 0;

 private:
  const std::string mDesc;
  std::ostringstream mFailStream;

  static std::vector<Test*> sTests;
  friend struct RunTests;
};

} // namespace

#endif BCI_TEST_H
