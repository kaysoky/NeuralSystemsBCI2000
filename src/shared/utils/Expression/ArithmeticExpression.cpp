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
#include <limits>
#include "ArithmeticExpression.h"
#include "BCIError.h"
#include "BCIException.h"
#include "ClassName.h"

using namespace std;
using namespace bci;
using namespace ExpressionParser;

static const pair<string, double> sConstants[] =
{
  make_pair( "pi", 4.0 * ::atan( 1.0 ) ),
  make_pair( "e", ::exp( 1.0 ) ),
  make_pair( "inf", numeric_limits<double>::infinity() ),
  make_pair( "nan", numeric_limits<double>::quiet_NaN() ),
  make_pair( "true", 1 ),
  make_pair( "false", 0 ),
};
const ArithmeticExpression::VariableContainer
ArithmeticExpression::Constants( &sConstants[0], &sConstants[sizeof( sConstants ) / sizeof( *sConstants )] );

ArithmeticExpression::ArithmeticExpression( const std::string& s )
: mExpression( s ),
  mThrowOnError( false ),
  mCompilationState( none )
{
}

ArithmeticExpression::ArithmeticExpression( const ArithmeticExpression& e )
: mExpression( e.mExpression ),
  mCompilationState( none )
{
}

ArithmeticExpression::~ArithmeticExpression()
{
  Cleanup();
  ClearStatements();
}

const ArithmeticExpression&
ArithmeticExpression::operator=( const ArithmeticExpression& e )
{
  mExpression = e.mExpression;
  mInput.str( "" );
  mInput.clear();
  mErrors.str( "" );
  mErrors.clear();
  mContext = Context();
  mCompilationState = none;
  Cleanup();
  ClearStatements();
  return *this;
}

bool
ArithmeticExpression::Compile( const Context& inContext )
{
  mContext = inContext;
  mCompilationState = Parse() ? success : attempted;
  ReportErrors();
  return mCompilationState == success;
}

bool
ArithmeticExpression::IsValid( const Context& inContext )
{
  mErrors.clear();
  mErrors.str( "" );
  mContext = inContext;
  if( mContext.variables )
    mContext.variables = new VariableContainer( *mContext.variables );
  if( Parse() )
    DoEvaluate();
  delete mContext.variables;
  mContext = Context();
  bool result = mErrors.str().empty();
  mErrors.clear();
  mErrors.str( "" );
  return result;
}

double
ArithmeticExpression::Evaluate()
{
  double result = 0;
  if( mCompilationState == none )
    Compile();
  if( mCompilationState == success )
    result = DoEvaluate();
  ReportErrors();
  return result;
}

double
ArithmeticExpression::DoEvaluate()
{
  double result = 0;
  try
  {
    for( size_t i = 0; i < mStatements.size(); ++i )
      result = mStatements[i]->Evaluate();
  }
  catch( const exception& e )
  {
    Errors() << e.what() << endl;
  }
#ifdef VCL_EXCEPTIONS
  catch( const class EMathError& e )
  {
    Errors() << e.Message.c_str() << endl;
  }
#endif
  return result;
}

void
ArithmeticExpression::Add( Node* inpNode )
{
  if( inpNode )
    mStatements.push_back( inpNode );
}

Node*
ArithmeticExpression::Variable( const string& inName )
{
  Node* result = NULL;
  if( mContext.constants )
  {
    VariableContainer::const_iterator i = mContext.constants->find( inName );
    if( i != mContext.constants->end() )
      result = new ConstantNode( i->second );
  }
  if( !result && mContext.variables )
  {
    VariableContainer::iterator i = mContext.variables->find( inName );
    if( i != mContext.variables->end() )
      result = new VariableNode( i->second );
  }
  if( !result )
    Errors() << "Variable \"" << inName << "\" does not exist" << endl;
  return result;
}

Node*
ArithmeticExpression::VariableAssignment( const std::string& inName, Node* inRHS )
{
  Node* result = NULL;
  if( mContext.constants &&  mContext.constants->find( inName ) !=  mContext.constants->end() )
    Errors() << inName << ": Not assignable" << endl;
  else if( ! mContext.variables )
    Errors() << inName << ": Cannot create variables" << endl;
  else
    result = new AssignmentNode( ( *mContext.variables )[inName], inRHS );
  return result;
}

Node*
ArithmeticExpression::Function( const std::string& inName, const NodeList& inArguments )
{
  Node* result = NULL;

  #define CONSTFUNC0( x )  { #x, true, 0, ( void* )static_cast<FunctionNode<0>::Pointer>( &::x ) },
  #define CONSTFUNC1( x )  { #x, true, 1, ( void* )static_cast<FunctionNode<1>::Pointer>( &::x ) },
  #define CONSTFUNC2( x )  { #x, true, 2, ( void* )static_cast<FunctionNode<2>::Pointer>( &::x ) },
  #define CONSTFUNC3( x )  { #x, true, 3, ( void* )static_cast<FunctionNode<3>::Pointer>( &::x ) },

  static const struct
  {
    const char* name;
    bool        isConst; // same arguments give always the same result (e.g., for rand() this would be false)
    size_t      numArgs;
    void*       function;
  }
  functions[] =
  {
    CONSTFUNC1( sqrt )

    CONSTFUNC1( fabs )
    { "abs", true, 1, ( void* )static_cast<FunctionNode<1>::Pointer>( &::fabs ) },

    CONSTFUNC2( fmod )
    { "mod", true, 2, ( void* )static_cast<FunctionNode<2>::Pointer>( &::fmod ) },

    CONSTFUNC1( floor )  CONSTFUNC1( ceil )

    CONSTFUNC1( exp )    CONSTFUNC1( log )    CONSTFUNC1( log10 )
    CONSTFUNC2( pow )

    CONSTFUNC1( sin )    CONSTFUNC1( cos )    CONSTFUNC1( tan )
    CONSTFUNC1( asin )   CONSTFUNC1( acos )   CONSTFUNC1( atan )   CONSTFUNC2( atan2 )
    CONSTFUNC1( sinh )   CONSTFUNC1( cosh )   CONSTFUNC1( tanh )

  };
  static const size_t numFunctions = sizeof( functions ) / sizeof( *functions );
  size_t i = 0;
  while( i < numFunctions && inName != functions[i].name )
    ++i;
  if( i >= numFunctions )
  {
    Errors() << inName << "(): Unknown function" << endl;
  }
  else if( functions[i].numArgs != inArguments.size() )
  {
    Errors() << inName << "(): Wrong number of arguments, expected "
             << functions[i].numArgs << ", got " 
             << inArguments.size()
             << endl;
  }
  else
  {
    switch( functions[i].numArgs )
    {
      case 0:
        result = new FunctionNode<0>( functions[i].isConst, FunctionNode<0>::Pointer( functions[i].function ) );
        break;
      case 1:
        result = new FunctionNode<1>( functions[i].isConst, FunctionNode<1>::Pointer( functions[i].function ), inArguments[0] );
        break;
      case 2:
        result = new FunctionNode<2>( functions[i].isConst, FunctionNode<2>::Pointer( functions[i].function ), inArguments[0], inArguments[1] );
        break;
      case 3:
        result = new FunctionNode<3>( functions[i].isConst, FunctionNode<3>::Pointer( functions[i].function ), inArguments[0], inArguments[1], inArguments[2] );
        break;
      default:
        Errors() << inName << "(): Unsupported number of function arguments" << endl;
    }
  }
  return result;
}

Node*
ArithmeticExpression::MemberFunction( const string& inObject, const string&, const NodeList& )
{
  Errors() << inObject << ": Unknown object"
           << endl;
  return NULL;
}

Node*
ArithmeticExpression::Signal( AddressNode*, AddressNode* )
{
  Errors() << "Expressions of type " << ClassName( typeid( *this ) ) << " do not allow access to signals"
           << endl;
  return NULL;
}

Node*
ArithmeticExpression::State( const string& )
{
  Errors() << "Expressions of type " << ClassName( typeid( *this ) ) << " do not allow access to states"
           << endl;
  return NULL;
}

Node*
ArithmeticExpression::StateAssignment( const string& inName, Node* )
{
  return ArithmeticExpression::State( inName );
}


bool
ArithmeticExpression::Parse()
{
  ClearStatements();
  mInput.clear();
  mInput.str( mExpression );
  try
  {
    ExpressionParser::yyparse( this );
  }
  catch( const exception& e )
  {
    Errors() << e.what() << endl;
  }
#ifdef VCL_EXCEPTIONS
  catch( const class EMathError& e )
  {
    Errors() << e.Message.c_str() << endl;
  }
#endif
  catch( ... )
  {
    Errors() << "Unknown exception caught" << endl;
  }
  Cleanup();
  bool success = mErrors.str().empty();
  if( success )
    for( size_t i = 0; i < mStatements.size(); ++i )
      mStatements[i] = mStatements[i]->Simplify();
  else
    ClearStatements();
  return success;
}

void
ArithmeticExpression::ReportErrors()
{
  string errors = mErrors.str();
  ostringstream errorReport;
  if( !errors.empty() )
  {
    int numErrors = 0;
    for( size_t pos = errors.find( '\n' ); pos != string::npos; pos = errors.find( '\n', pos + 1 ) )
      ++numErrors;
    errors = errors.substr( 0, errors.length() - 1 ) + '.';
    if( numErrors > 1 )
      for( size_t pos = errors.find( '\n' ); pos != string::npos; pos = errors.find( '\n', pos + 3 ) )
        errors = errors.substr( 0, pos ) + ".\n * " + errors.substr( pos + 1 );
    errorReport << "When processing \""
                << mExpression << "\""
                << ( numErrors == 1 ? ": " : ", multiple errors occurred.\n * " )
                << errors;
  }
  mErrors.clear();
  mErrors.str( "" );
  if( !errorReport.str().empty() )
  {
    if( mThrowOnError )
      throw bciexception_( errorReport.str() );
    else
      bcierr_ << errorReport.str() << flush;
  }
}

void
ArithmeticExpression::Cleanup()
{
  while( !mAllocations.empty() )
  { 
    mAllocations.begin()->Delete();
    mAllocations.erase( mAllocations.begin() );
  }
}

void
ArithmeticExpression::ClearStatements()
{
  for( NodeList::reverse_iterator i = mStatements.rbegin(); i != mStatements.rend(); ++i )
    delete *i;
  mStatements.clear();
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
