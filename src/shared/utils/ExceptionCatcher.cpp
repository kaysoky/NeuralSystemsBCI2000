//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that simplifies high-level exception catching.
//   Call its Execute() function with a functor as argument in order
//   to execute the functor in a try block, catching exceptions that
//   occur during execution of the functor.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "ExceptionCatcher.h"
#include "BCIException.h"
#include "BCIError.h"
#include "OSError.h"
#include "ClassName.h"

using namespace std;

bool
ExceptionCatcher::Run( Runnable& inRunnable )
{
  // This function catches Win32 "structured" exceptions.
  // Handling of those cannot coexist with C++ exception handling in the same function,
  // so we need another function Run2() that handles C++ exceptions.
  bool result = false;
#if _MSC_VER
  __try
#endif // _MSC_VER
  {
    result = Run2( inRunnable );
  }
#if _MSC_VER
  // For breakpoint exceptions, we want to execute the default handler (which opens the debugger).
  __except( ::GetExceptionCode() == EXCEPTION_BREAKPOINT ? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER )
  {
    ReportWin32Exception( ::GetExceptionCode() );
  }
#endif // _MSC_VER
  return result;
}

bool
ExceptionCatcher::Run2( Runnable& inRunnable )
{
  // This function handles C++ exceptions.
  bool result = false;
  try
  {
    inRunnable.Run();
    result = true;
  }
  catch( const BCIException& e )
  {
    bcierr__ << e.what()
             << UserMessage()
             << endl;
  }
  catch( const exception& e )
  {
    bcierr__ << "Unhandled exception of type "
             << bci::ClassName( typeid( e ) )
             << ": " << e.what()
             << UserMessage()
             << endl;
  }
#if defined( SystemHPP ) && !defined( _NO_VCL ) // VCL is available both at compile and link time
  catch( Exception& e )
  {
    bcierr__ << "Unhandled exception of type "
             << bci::ClassName( typeid( e ) )
             << ": " << AnsiString( e.Message ).c_str()
             << UserMessage()
             << endl;
  }
#endif // SystemHPP  && !_NO_VCL
  return result;
}

#if _MSC_VER
# define EXCEPTION( x ) { EXCEPTION_##x, #x },
void
ExceptionCatcher::ReportWin32Exception( int inCode )
{
  static const struct
  {
    int code;
    const char* description;
  }
  exceptions[] =
  {
    EXCEPTION( ACCESS_VIOLATION )
    EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
    EXCEPTION( BREAKPOINT )
    EXCEPTION( DATATYPE_MISALIGNMENT )
    EXCEPTION( FLT_DENORMAL_OPERAND )
    EXCEPTION( FLT_DIVIDE_BY_ZERO )
    EXCEPTION( FLT_INEXACT_RESULT )
    EXCEPTION( FLT_INVALID_OPERATION )
    EXCEPTION( FLT_OVERFLOW )
    EXCEPTION( FLT_STACK_CHECK )
    EXCEPTION( FLT_UNDERFLOW )
    EXCEPTION( GUARD_PAGE )
    EXCEPTION( ILLEGAL_INSTRUCTION )
    EXCEPTION( IN_PAGE_ERROR )
    EXCEPTION( INT_DIVIDE_BY_ZERO )
    EXCEPTION( INT_OVERFLOW )
    EXCEPTION( INVALID_DISPOSITION )
    EXCEPTION( INVALID_HANDLE )
    EXCEPTION( NONCONTINUABLE_EXCEPTION )
    EXCEPTION( PRIV_INSTRUCTION )
    EXCEPTION( SINGLE_STEP )
    EXCEPTION( STACK_OVERFLOW )
  };
  static const size_t numExceptions = sizeof( exceptions ) / sizeof( *exceptions );
  size_t i = 0;
  while( i < numExceptions && exceptions[i].code != inCode )
    ++i;
  const char* pDescription = i < numExceptions ? exceptions[i].description : "<n/a>";

  bcierr__ << "Unhandled Win32 exception 0x"
           << hex << inCode << ": "
           << pDescription
           << UserMessage()
           << endl;
}
#endif // _MSC_VER

string
ExceptionCatcher::UserMessage() const
{
  string result;
  if( !mMessage.empty() )
    result += "\n" + mMessage;
  return result;
}
