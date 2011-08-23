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

using namespace std;

ArithmeticExpression::ArithmeticExpression()
: mValue( 0 )
{
}

ArithmeticExpression::ArithmeticExpression( const std::string& s )
: mExpression( s ), mValue( 0 )
{
}

ArithmeticExpression::ArithmeticExpression( const ArithmeticExpression& e )
: mExpression( e.mExpression ), mValue( 0 )
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
  return *this;
}

double
ArithmeticExpression::Evaluate()
{
  Parse();
  if( !mErrors.str().empty() )
    bcierr__ << "ArithmeticExpression::Evaluate: When evaluating \""
             << mExpression << "\": "
             << mErrors.str() << endl;
  return mValue;
}

bool
ArithmeticExpression::IsValid()
{
  Parse();
  return mErrors.str().empty();
}

double
ArithmeticExpression::State( const string& )
{
  Errors() << "Use an Expression rather than an ArithmeticExpression to access states"
           << endl;
  return 0;
}

double
ArithmeticExpression::Signal( const string&, const string& )
{
  Errors() << "Use an Expression rather than an ArithmeticExpression to access a signal"
           << endl;
  return 0;
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

string*
ArithmeticExpression::AllocateCopy( const string& s )
{
  string* p = new string( s );
  mAllocatedStrings.push_front( p );
  return p;
}

void
ArithmeticExpression::Cleanup()
{
  while( !mAllocatedStrings.empty() )
  {
    delete mAllocatedStrings.front();
    mAllocatedStrings.pop_front();
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
