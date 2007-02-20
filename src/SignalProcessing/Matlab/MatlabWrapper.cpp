////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: MatlabWrapper.cpp
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Nov 30, 2005
//
// $Log$
// Revision 1.1  2005/12/20 11:38:07  mellinger
// Initial version.
//
//
// Description: Wrapper classes for convenient creation and manipulation of
//              Matlab workspace variables, and calling of Matlab functions.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "MatlabWrapper.h"
#include "UBCIError.h"

#include <sstream>

using namespace std;

static const string cErrorVariable = "bci_Error";

////////////////////////////////////////////////////////////////////////////////
// MatlabEngine::DoubleMatrix definitions                                     //
////////////////////////////////////////////////////////////////////////////////
MatlabEngine::DoubleMatrix::DoubleMatrix( const GenericSignal& inSignal )
: vector<vector<double> >( inSignal.Channels(), vector<double>( inSignal.Elements() ) )
{
  for( size_t channel = 0; channel < size(); ++channel )
    for( size_t sample = 0; sample < ( *this )[ channel ].size(); ++sample )
      ( *this )[ channel ][ sample ] = inSignal( channel, sample );
}

MatlabEngine::DoubleMatrix::operator GenericSignal() const
{
  GenericSignal signal( size(), empty() ? 0 : ( *this )[ 0 ].size(), SignalType::float32 );
  for( size_t channel = 0; channel < size(); ++channel )
    for( size_t sample = 0; sample < ( *this )[ channel ].size(); ++sample )
      signal( channel, sample ) = ( *this )[ channel ][ sample ];
  return signal;
}

MatlabEngine::DoubleMatrix::DoubleMatrix( const SignalProperties& inProperties )
: vector<vector<double> >( 1, vector<double>( 2 ) )
{
  ( *this )[ 0 ][ 0 ] = inProperties.Channels();
  ( *this )[ 0 ][ 1 ] = inProperties.Elements();
}

MatlabEngine::DoubleMatrix::operator SignalProperties() const
{
  size_t channels = 0,
         elements = 0;
  if( !empty() )
  {
    if( !( *this )[ 0 ].empty() )
      channels = ( *this )[ 0 ][ 0 ];
    if( ( *this )[ 0 ].size() > 1 )
      elements = ( *this )[ 0 ][ 1 ];
  }
  return SignalProperties( channels, elements, SignalType::float32 );
}

////////////////////////////////////////////////////////////////////////////////
// MatlabEngine::StringMatrix definitions                                     //
////////////////////////////////////////////////////////////////////////////////
MatlabEngine::StringMatrix::StringMatrix( const PARAM& p )
: vector<vector<string> >( p.GetNumRows(), vector<string>( p.GetNumColumns() ) )
{
  for( size_t row = 0; row < size(); ++row )
    for( size_t column = 0; column < ( *this )[ row ].size(); ++column )
      ( *this )[ row ][ column ] = p.GetValue( row, column );
}

////////////////////////////////////////////////////////////////////////////////
// MatlabEngine definitions                                                   //
////////////////////////////////////////////////////////////////////////////////
int MatlabEngine::sNumInstances = 0;
Engine* MatlabEngine::spEngineRef = NULL;

// Matlab Engine DLL imports
const char* MatlabEngine::sLibEngName = "libeng";
Engine*  ( *MatlabEngine::engOpen )( const char* ) = NULL;
int      ( *MatlabEngine::engClose )( Engine* ) = NULL;
int      ( *MatlabEngine::engEvalString )( Engine*, const char* ) = NULL;
mxArray* ( *MatlabEngine::engGetVariable )( Engine*, const char* ) = NULL;
int      ( *MatlabEngine::engPutVariable )( Engine*, const char*, const mxArray* ) = NULL;

// Matlab MX DLL imports
const char* MatlabEngine::sLibMxName = "libmx";
mxArray* ( *MatlabEngine::mxCreateString )( const char* ) = NULL;
char*    ( *MatlabEngine::mxArrayToString )( const mxArray* ) = NULL;
mxArray* ( *MatlabEngine::mxCreateCellMatrix )( int, int ) = NULL;
mxArray* ( *MatlabEngine::mxGetCell )( const mxArray*, int ) = NULL;
void     ( *MatlabEngine::mxSetCell )( mxArray*, int, mxArray* ) = NULL;

mxArray* ( *MatlabEngine::mxCreateNumericMatrix )( int, int, mxClassID, int ) = NULL;
double*  ( *MatlabEngine::mxGetPr )( const mxArray* ) = NULL;
void     ( *MatlabEngine::mxSetPr )( mxArray*, double* ) = NULL;

int      ( *MatlabEngine::mxGetNumberOfDimensions )( const mxArray* ) = NULL;
const int* ( *MatlabEngine::mxGetDimensions )( const mxArray* ) = NULL;
int      ( *MatlabEngine::mxCalcSingleSubscript )( const mxArray*, int, const int* ) = NULL;

void     ( *MatlabEngine::mxDestroyArray )( mxArray* ) = NULL;
void     ( *MatlabEngine::mxFree )( void* ) = NULL;

// Imports name table
#define PROC( x )   { ( void** )&(x), #x },
MatlabEngine::ProcNameEntry MatlabEngine::sEngProcNames[] =
{
  // Matlab Engine
  PROC( engOpen )
  PROC( engClose )
  PROC( engEvalString )
  PROC( engGetVariable )
  PROC( engPutVariable )
};

MatlabEngine::ProcNameEntry MatlabEngine::sMxProcNames[] =
{
  // Matlab MX
  PROC( mxCreateString )
  PROC( mxArrayToString )
  PROC( mxCreateCellMatrix )
  PROC( mxGetCell )
  PROC( mxSetCell )
  PROC( mxCreateNumericMatrix )
  PROC( mxGetPr )
  PROC( mxSetPr )
  PROC( mxGetNumberOfDimensions )
  PROC( mxGetDimensions )
  PROC( mxCalcSingleSubscript )
  PROC( mxDestroyArray )
  PROC( mxFree )
};

MatlabEngine::MatlabEngine()
{
  ++sNumInstances;
  if( sNumInstances == 1 && !spEngineRef )
  {
    // Load libraries.
    if( LoadDLL( sLibEngName, sizeof( sEngProcNames ) / sizeof( *sEngProcNames ), sEngProcNames )
        && LoadDLL( sLibMxName, sizeof( sMxProcNames ) / sizeof( *sMxProcNames ), sMxProcNames ) )
    { // Open the Matlab engine.
      spEngineRef = engOpen( NULL );
      if( !spEngineRef )
        bcierr << "Could not open Matlab engine" << endl;
      else
        PutString( cErrorVariable, "" );
    }
  }
}

MatlabEngine::~MatlabEngine()
{
  --sNumInstances;
  if( sNumInstances < 1 && spEngineRef )
  {
    ClearVariable( cErrorVariable );
    engClose( spEngineRef );
    spEngineRef = NULL;
  }
}

bool
MatlabEngine::CreateGlobal( const string& inName )
{
  string command;
  command += "global " + inName + ";";
  return ( 0 == engEvalString( spEngineRef, command.c_str() ) );
}

bool
MatlabEngine::ClearVariable( const string& inName )
{
  string command;
  command += "clear " + inName + "; clear global " + inName + ";";
  return ( 0 == engEvalString( spEngineRef, command.c_str() ) );
}

string
MatlabEngine::GetString( const string& inExp )
{
  string result;
  mxArray* ans = GetMxArray( inExp );
  if( ans )
  {
    const char* s = mxArrayToString( ans );
    if( s == NULL )
      bcierr << "Could not read \"" << inExp << "\" from Matlab workspace" << endl;
    else
      result = s;
    mxFree( ( void* )s );
    mxDestroyArray( ans );
  }
  return result;
}

bool
MatlabEngine::PutString( const string& inExp, const string& inValue )
{
  mxArray* val = mxCreateString( inValue.c_str() );
  bool success = PutMxArray( inExp, val );
  mxDestroyArray( val );
  return success;
}

double
MatlabEngine::GetScalar( const string& inExp )
{
  DoubleMatrix result = GetMatrix( inExp );
  if( result.empty() || result[ 0 ].empty() )
  {
    bcierr << "Could not get scalar value \"" << inExp << "\"" << endl;
    result.resize( 1, vector<double>( 1 ) );
  }
  return result[ 0 ][ 0 ];
}

bool
MatlabEngine::PutScalar( const string& inExp, double inValue )
{
  return PutMatrix( inExp, inValue );
}

MatlabEngine::DoubleMatrix
MatlabEngine::GetMatrix( const string& inExp )
{
  DoubleMatrix result;
  mxArray* ans = GetMxArray( inExp );
  if( ans )
  {
    int numDims = mxGetNumberOfDimensions( ans );
    const int* dims = mxGetDimensions( ans );
    if( numDims != 2 )
      bcierr << "Can only handle two dimensions" << endl;
    result.resize( dims[ 0 ], vector<double>( dims[ 1 ] ) );
    double* value = mxGetPr( ans );
    if( value )
    {
      int indices[] = { 0, 0 };
      for( size_t i = 0; i < result.size(); ++i )
      {
        indices[ 0 ] = i;
        for( size_t j = 0; j < result[ i ].size(); ++j )
        {
          indices[ 1 ] = j;
          result[ i ][ j ] = value[ mxCalcSingleSubscript( ans, 2, indices ) ];
        }
      }
    }
    mxDestroyArray( ans );
  }
  return result;
}

bool
MatlabEngine::PutMatrix( const string& inExp, const DoubleMatrix& inValue )
{
  int sizeDim2 = inValue.empty() ? 0 : inValue[ 0 ].size();
  mxArray* val = mxCreateNumericMatrix( inValue.size(), sizeDim2, mxDOUBLE_CLASS, mxREAL );
  double* data = mxGetPr( val );
  if( data )
  {
    int indices[] = { 0, 0 };
    for( size_t i = 0; i < inValue.size(); ++i )
    {
      indices[ 0 ] = i;
      for( size_t j = 0; j < inValue[ i ].size(); ++j )
      {
        indices[ 1 ] = j;
        data[ mxCalcSingleSubscript( val, 2, indices ) ] = inValue[ i ][ j ];
      }
    }
  }
  bool success = PutMxArray( inExp, val );
  mxDestroyArray( val );
  return success;
}

MatlabEngine::StringMatrix
MatlabEngine::GetCells( const string& inExp )
{
  StringMatrix result;
  mxArray* ans = GetMxArray( inExp );
  if( ans )
  {
    int numDims = mxGetNumberOfDimensions( ans );
    const int* dims = mxGetDimensions( ans );
    if( numDims != 2 )
      bcierr << "Can only handle two dimensions" << endl;
    result.resize( dims[ 0 ], vector<string>( dims[ 1 ] ) );
    int indices[] = { 0, 0 };
    for( size_t i = 0; i < result.size(); ++i )
    {
      indices[ 0 ] = i;
      for( size_t j = 0; j < result[ i ].size(); ++j )
      {
        indices[ 1 ] = j;
        int idx = mxCalcSingleSubscript( ans, 2, indices );
        mxArray* cell = mxGetCell( ans, idx );
        if( cell )
        {
          const char* s = mxArrayToString( cell );
          if( s == NULL )
            bcierr << "Could not read string value \"" << inExp << "\"" << endl;
          else
          {
            result[ i ][ j ] = s;
            mxFree( ( void* )s );
          }
        }
      }
    }
    mxDestroyArray( ans );
  }
  return result;
}

bool
MatlabEngine::PutCells( const string& inExp, const StringMatrix& inValue )
{
  int sizeDim2 = inValue.empty() ? 0 : inValue[ 0 ].size();
  mxArray* mat = mxCreateCellMatrix( inValue.size(), sizeDim2 );
  int indices[] = { 0, 0 };
  for( size_t i = 0; i < inValue.size(); ++i )
  {
    indices[ 0 ] = i;
    for( size_t j = 0; j < inValue[ i ].size(); ++j )
    {
      indices[ 1 ] = j;
      int idx = mxCalcSingleSubscript( mat, 2, indices );
      mxDestroyArray( mxGetCell( mat, idx ) );
      mxArray* val = mxCreateString( inValue[ i ][ j ].c_str() );
      mxSetCell( mat, idx, val );
    }
  }
  bool success = PutMxArray( inExp, mat );
  mxDestroyArray( mat );
  return success;
}

::mxArray*
MatlabEngine::GetMxArray( const string& inExp )
{
  mxArray* ans = NULL;
  if( 0 == engEvalString( spEngineRef, ( string( "ans=" ) + inExp ).c_str() ) )
    ans = engGetVariable( spEngineRef, "ans" );
  if( !ans )
    bcierr << "Could not read \"" << inExp << "\" from Matlab workspace" << endl;
  return ans;
}

bool
MatlabEngine::PutMxArray( const string& inExp, const mxArray* inArray )
{
  bool success = ( 0 == engPutVariable( spEngineRef, "ans", inArray ) );
  if( success )
    success = ( 0 == engEvalString( spEngineRef, ( inExp + "=ans;" ).c_str() ) );
  if( !success )
    bcierr << "Could not put value into \"" << inExp << "\"" << endl;
  return success;
}

const std::string&
MatlabEngine::OSError( long inErrorCode )
{
  static string message;
  char* buf = NULL;
  ::FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    inErrorCode,
    0, // Default language
    reinterpret_cast<char*>( &buf ),
    0,
    NULL
  );
  if( buf )
  {
    message = buf;
    ::LocalFree( buf );
  }
  else
  {
    ostringstream oss;
    oss << "Unknown error " << inErrorCode;
    message = oss.str();
  }
  message = message.substr( 0, message.find_last_of( "\n" ) - 1 );
  return message;
}

bool
MatlabEngine::LoadDLL( const char* inName, int inNumProcs, ProcNameEntry* inProcNames )
{
  {
    // According to Mathworks Helpdesk, Solution Number: 1-1134M0,
    // this code works around a bug in Matlab R14Sp1 and R14Sp2
    // by resetting the FPU.
    static unsigned short CtrlWord = 0x123f;
    __asm  fninit;             // initialize fpu
    __asm  fnclex;             // clear fpu exceptions
    __asm  fldcw CtrlWord;     // load fpu control word
  }
  bool success = true;
  void* dllHandle = ::LoadLibrary( inName );
  if( !dllHandle )
  {
    success = false;
    long err = ::GetLastError();
    bcierr << "Could not load libary " << inName << ":\n"
           << OSError( err ) << endl;
  }
  else
  {
    for( int i = 0; i < inNumProcs; ++i )
    {
      void* address = ::GetProcAddress( dllHandle, inProcNames[ i ].mName );
      if( !address )
      {
        success = false;
        long err = ::GetLastError();
        bcierr << "Could not get address of " << inProcNames[ i ].mName << ":\n"
               << OSError( err ) << endl;
      }
      *inProcNames[ i ].mProc = address;
    }
  }
  return success;
}

////////////////////////////////////////////////////////////////////////////////
// MatlabFunction definitions                                                 //
////////////////////////////////////////////////////////////////////////////////
MatlabFunction::MatlabFunction( const string& inName )
: mName( inName ),
  mExists( false )
  {
  // Check whether there exists an M-file with the required name in the Matlab
  // search path.
  string command;
  command += "exist('" + inName + "')";
  if( engEvalString( spEngineRef, command.c_str() ) == 0 )
  {
    mxArray* ans = engGetVariable( spEngineRef, "ans" );
    if( ans )
    {
      double* value = mxGetPr( ans );
      mExists = ( value[ 0 ] == 2 );
      mxDestroyArray( ans );
    }
  }
}

MatlabFunction::~MatlabFunction()
{
}

MatlabFunction&
MatlabFunction::InputArgument( const string& inArg )
{
  mInputArguments.push_back( inArg );
  return *this;
}

MatlabFunction&
MatlabFunction::OutputArgument( const string& inArg )
{
  mOutputArguments.push_back( inArg );
  return *this;
}

const string&
MatlabFunction::Execute() const
{
  // Execute the following commands and return the content of errorvar:
  //
  // errorvar = '';
  // try
  //   [out1,...]=function(in1,...);
  // catch
  //   errorvar=lasterr;
  // end;
  
  static string result;
  result = "";
  if( !mExists )
    result = "Function " + mName + " does not exist in the Matlab search path.";
  else
  {
    ostringstream command;
    command << cErrorVariable << "=''; try ";
    if( !mOutputArguments.empty() )
    {
      command << "[" << mOutputArguments[ 0 ];
      for( size_t i = 1; i < mOutputArguments.size(); ++i )
        command << "," << mOutputArguments[ i ];
      command << "]=";
    }
    command << mName;
    if( !mInputArguments.empty() )
    {
      command << "(" << mInputArguments[ 0 ];
      for( size_t i = 1; i < mInputArguments.size(); ++i )
        command << "," << mInputArguments[ i ];
      command << ")";
    }
    command << "; catch " << cErrorVariable << "=lasterr; end;";
    if( 0 != engEvalString( spEngineRef, command.str().c_str() ) )
      result = "Could not execute Matlab command:\n" + command.str();
    else
    {
      result = MatlabEngine::GetString( cErrorVariable );
      MatlabEngine::PutString( cErrorVariable, "" );
    }
  }
  return result;
}


