//////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A simple expression parser for use within BCI2000.
//   ArithmeticExpression provides expression parsing; its Expression
//   descendant, in addition, allows access to State and Signal values.
//   For details about expression syntax, see Expression.h.
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
//////////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#if( defined( SystemHPP ) && !defined( _NO_VCL ) )
# define VCL_EXCEPTIONS 1
#endif

#include <cmath>
#include <string>
#include <stdexcept>
#include <iostream>
#include "ArithmeticExpression.h"
#include "BCIError.h"
#include "ClassName.h"

using namespace std;
using namespace bci;

ArithmeticExpression::ArithmeticExpression()
: mValue( 0 ),
  mpVariables( NULL )
{
}

ArithmeticExpression::ArithmeticExpression( const std::string& s )
: mExpression( s ),
  mValue( 0 ),
  mpVariables( NULL )
{
}

ArithmeticExpression::ArithmeticExpression( const ArithmeticExpression& e )
: mExpression( e.mExpression ),
  mValue( 0 ),
  mpVariables( NULL )
{
}

ArithmeticExpression::~ArithmeticExpression()
{
  Cleanup();
}

const ArithmeticExpression&
ArithmeticExpression::operator=( const ArithmeticExpression& e )
{
  mExpression = e.mExpression;
  mInput.str( "" );
  mInput.clear();
  mErrors.str( "" );
  mErrors.clear();
  mValue = 0;
  mpVariables = NULL;
  return *this;
}

double
ArithmeticExpression::Evaluate( VariableContainer* iopVariables )
{
  mpVariables = iopVariables;
  Parse();
  mpVariables = NULL;
  if( !mErrors.str().empty() )
    bcierr__ << "ArithmeticExpression::Evaluate: When evaluating \""
             << mExpression << "\": "
             << mErrors.str() << endl;
  return mValue;
}

bool
ArithmeticExpression::IsValid( const VariableContainer* inpVariables )
{
  if( inpVariables )
    mpVariables = new VariableContainer( *inpVariables );
  else
    mpVariables = NULL;
  Parse();
  delete mpVariables;
  return mErrors.str().empty();
}

double
ArithmeticExpression::Variable( const string& inName )
{
  double result = 0;
  if( !mpVariables || mpVariables->find( inName ) == mpVariables->end() )
    Errors() << "Variable \"" << inName << "\" does not exist" << endl;
  else
    result = ( *mpVariables )[inName];
  return result;
}

void
ArithmeticExpression::VariableAssignment( const std::string& inName, double inValue )
{
  if( !mpVariables )
    Errors() << inName << ": Cannot create variable" << endl;
  else
    ( *mpVariables )[inName] = inValue;
}

double
ArithmeticExpression::MemberVariable( double, const string& inName )
{
  Errors() << inName << ": Expressions of type " << ClassName( typeid( *this ) ) << " do not allow member variables"
           << endl;
  return 0;
}

void
ArithmeticExpression::MemberVariableAssignment( double, const std::string& inName, double )
{
  ArithmeticExpression::MemberVariable( 0, inName ); // reports error
}

double
ArithmeticExpression::Function( const std::string& inName, const ArgumentList& inArguments )
{
  double result = 0;

  typedef double (*func0Ptr)();
  typedef double (*func1Ptr)( double );
  typedef double (*func2Ptr)( double, double );
  typedef double (*func3Ptr)( double, double, double );

  #define FUNC0( x )  { #x, 0, static_cast<func0Ptr>( &::x ) },
  #define FUNC1( x )  { #x, 1, static_cast<func1Ptr>( &::x ) },
  #define FUNC2( x )  { #x, 2, static_cast<func2Ptr>( &::x ) },
  #define FUNC3( x )  { #x, 2, static_cast<func3Ptr>( &::x ) },

  static const struct
  {
    const char* name;
    int         nargs;
    void*       fptr;
  }
  functions[] =
  {
    FUNC1( sqrt )

    FUNC1( fabs )
    { "abs", 1, static_cast<func1Ptr>( &::fabs ) },

    FUNC2( fmod )
    { "mod", 2, static_cast<func2Ptr>( &::fmod ) },

    FUNC1( floor )  FUNC1( ceil )

    FUNC1( exp )    FUNC1( log )    FUNC1( log10 )
    FUNC2( pow )

    FUNC1( sin )    FUNC1( cos )    FUNC1( tan )
    FUNC1( asin )   FUNC1( acos )   FUNC1( atan )   FUNC2( atan2 )  
    FUNC1( sinh )   FUNC1( cosh )   FUNC1( tanh )

  };
  static const int numFunctions = sizeof( functions ) / sizeof( *functions );
  size_t i = 0;
  while( i < numFunctions && inName != functions[i].name )
    ++i;
  if( i >= numFunctions )
  {
    Errors() << inName << "(): Unknown function" << endl;
  }
  else if( functions[i].nargs != inArguments.size() )
  {
    Errors() << inName << "(): Wrong number of arguments, expected " << functions[i].nargs << ", got " << inArguments.size() << endl;
  }
  else
  {
    switch( functions[i].nargs )
    {
      case 0:
        result = func0Ptr( functions[i].fptr )();
        break;
      case 1:
        result = func1Ptr( functions[i].fptr )( inArguments[0] );
        break;
      case 2:
        result = func2Ptr( functions[i].fptr )( inArguments[0], inArguments[1] );
        break;
      case 3:
        result = func3Ptr( functions[i].fptr )( inArguments[0], inArguments[1], inArguments[2] );
        break;
      default:
        Errors() << inName << "(): Unsupported number of function arguments" << endl;
    }
  }
  return result;
}

double
ArithmeticExpression::MemberFunction( double, const std::string& inName, const ArgumentList& )
{
  Errors() << inName << "(): Expressions of type " << ClassName( typeid( *this ) ) << " do not allow member functions"
           << endl;
  return 0;
}

double
ArithmeticExpression::Signal( const string&, const string& )
{
  Errors() << "Expressions of type " << ClassName( typeid( *this ) ) << " do not allow access to signals"
           << endl;
  return 0;
}

double
ArithmeticExpression::State( const string& )
{
  Errors() << "Expressions of type " << ClassName( typeid( *this ) ) << " do not allow access to states"
           << endl;
  return 0;
}

void
ArithmeticExpression::StateAssignment( const string& inName, double )
{
  ArithmeticExpression::State( inName );
}


void
ArithmeticExpression::Parse()
{
  mInput.clear();
  mInput.str( mExpression );
  mErrors.clear();
  mErrors.str( "" );
  try
  {
    if( ExpressionParser::yyparse( this ) != 0 )
      mValue = 0;
  }
  catch( const char* s )
  {
    Errors() << s << endl;
    mValue = 0;
  }
  catch( const exception& e )
  {
    Errors() << e.what() << endl;
    mValue = 0;
  }
#ifdef VCL_EXCEPTIONS
  catch( const class EMathError& e )
  {
    Errors() << e.Message.c_str() << endl;
    mValue = 0;
  }
#endif
  catch( ... )
  {
    Errors() << "Unknown exception caught" << endl;
    mValue = 0;
  }
  Cleanup();
}

void
ArithmeticExpression::Cleanup()
{
  while( !mAllocations.empty() )
  { // Deallocation in reverse order should prevent fragmentation.
    mAllocations.back()->Delete();
    mAllocations.pop_back();
  }
}

#ifdef VCL_EXCEPTIONS
// _matherr() error handling interface.
static void
ThrowMathError( const char* functionName )
{
  throw runtime_error( string( "Error in function " ) + functionName + "()" );
}

int
_matherr( struct _exception* e )
{
  ThrowMathError( e->name );
  return true;
}

int
_matherrl( struct _exceptionl* e )
{
  ThrowMathError( e->name );
  return true;
}
#endif // VCL_EXCEPTIONS
