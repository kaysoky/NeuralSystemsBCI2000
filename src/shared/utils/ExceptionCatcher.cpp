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
#include "PCHIncludes.h"
#pragma hdrstop

#include "ExceptionCatcher.h"
#include "BCIException.h"
#include "BCIError.h"
#include "OSError.h"
#include "ClassName.h"

#define HANDLE_SIGNALS ( !defined( _MSC_VER ) && !defined( __BORLANDC__ ) )
#if HANDLE_SIGNALS
# include <signal.h>
# include <setjmp.h>
#endif // HANDLE_SIGNALS

using namespace std;

#if HANDLE_SIGNALS
namespace {
#if __MINGW__ || __MINGW32__
  struct sigaction { void(*sa_handler)( int ); };
  void sigaction( int code, struct sigaction* pNew, struct sigaction* pOld )
  {
    if( pOld )
      pOld->sa_handler = ::signal( code, pNew->sa_handler );
    else
      ::signal( code, pNew->sa_handler );
  }
#endif // MINGW
  static struct
  {
    const char* name;
    const int code;
    struct sigaction action;
  }
  sSignals[] =
  {
    #define SIGNAL(x) { #x, x, {} }
    SIGNAL( SIGFPE ),
    SIGNAL( SIGILL ),
    SIGNAL( SIGSEGV ),
#if !defined( __MINGW__ ) && !defined( __MINGW32__ )
    SIGNAL( SIGBUS ),
#endif
  };
  static jmp_buf sCatchSignals;
  void SignalHandler( int inSignal )
  {
    ::longjmp( sCatchSignals, inSignal );
  }
  void InstallSignalHandlers()
  {
    struct sigaction action = { 0 };
    for( size_t i = 0; i < sizeof( sSignals ) / sizeof( *sSignals ); ++i )
    {
      action.sa_handler = &SignalHandler;
      sigaction( sSignals[i].code, &action, &sSignals[i].action );
    }
  }
  void UninstallSignalHandlers()
  {
    for( size_t i = 0; i < sizeof( sSignals ) / sizeof( *sSignals ); ++i )
      sigaction( sSignals[i].code, &sSignals[i].action, NULL );
  }
}
#endif // !_WIN32

#if _MSC_VER
bool
ExceptionCatcher::Run( Runnable& inRunnable )
{
  // This function catches Win32 "structured" exceptions.
  // Handling of those cannot coexist with C++ exception handling in the same function,
  // so we need another function Run2() that handles C++ exceptions.
  bool result = false;
  __try
  {
    result = Run2( inRunnable );
  }
  // For breakpoint exceptions, we want to execute the default handler (which opens the debugger).
  __except( ::GetExceptionCode() == EXCEPTION_BREAKPOINT ? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER )
  {
    ReportWin32Exception( ::GetExceptionCode() );
  }
  return result;
}
#elif HANDLE_SIGNALS
bool
ExceptionCatcher::Run( Runnable& inRunnable )
{
  bool result = false;
  static bool signalHandlingInstalled = false;
  if( signalHandlingInstalled )
  {
    result = Run2( inRunnable );
  }
  else
  {
    static OSMutex mutex;
    {
      OSMutex::Lock lock( mutex );
      signalHandlingInstalled = true;
      InstallSignalHandlers();
    }
    int signal = ::setjmp( sCatchSignals );
    if( signal == 0 )
      result = Run2( inRunnable );
    else
      ReportSignal( signal );
    {
      OSMutex::Lock lock( mutex );
      UninstallSignalHandlers();
      signalHandlingInstalled = false;
    }
  }
  return result;
}
#else // !_MSC_VER, !HANDLE_SIGNALS
bool
ExceptionCatcher::Run( Runnable& inRunnable )
{
  return Run2( inRunnable );
}
#endif


bool
ExceptionCatcher::Run2( Runnable& inRunnable )
{
  // This function handles C++ exceptions.
  string message;
  try
  {
    inRunnable.Run();
  }
  catch( const BCIException& e )
  {
    message = e.what() + UserMessage();
  }
  catch( const exception& e )
  {
    message = "Unhandled exception of type "
              + bci::ClassName( typeid( e ) )
              + ": " 
              + e.what()
              + UserMessage();
  }
#if defined( SystemHPP ) && !defined( _NO_VCL ) // VCL is available both at compile and link time
  catch( Exception& e )
  {
    message = "Unhandled exception of type "
              + bci::ClassName( typeid( e ) )
              + ": " 
              + AnsiString( e.Message ).c_str()
              + UserMessage();
  }
#endif // SystemHPP  && !_NO_VCL
#ifndef _MSC_VER
  catch( ... )
  {
    message = "Unknown unhandled exception " + UserMessage();
  }
#endif // _MSC_VER
  if( !message.empty() )
    OnReportException( message );
  return message.empty();
}

#if _MSC_VER
void
ExceptionCatcher::ReportWin32Exception( int inCode )
{
# define EXCEPTION( x ) { EXCEPTION_##x, #x },
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
  ostringstream oss;
  oss << "Unhandled Win32 exception 0x"
      << hex << inCode << ": "
      << pDescription
      << UserMessage();
  OnReportException( oss.str() );
}
#endif // _MSC_VER

#if HANDLE_SIGNALS
void
ExceptionCatcher::ReportSignal( int inSignal )
{
  size_t numSignals = sizeof( sSignals ) / sizeof( *sSignals ),
         i = 0;
  while( sSignals[i].code != inSignal )
    ++i;
  const char* pDescription = i < numSignals ? sSignals[i].name : "Unknown Signal";
  ostringstream oss;
  oss << "Signal caught: "
      << pDescription
      << UserMessage();
  OnReportException( oss.str() );
}
#endif // !HANDLE_SIGNALS

string
ExceptionCatcher::UserMessage() const
{
  string result;
  if( !mMessage.empty() )
    result += "\n" + mMessage;
  return result;
}

void
ExceptionCatcher::OnReportException( const string& inMessage )
{
  bcierr__ << inMessage << endl;
}
