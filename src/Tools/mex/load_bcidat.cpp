///////////////////////////////////////////////////////////////////////////////
// $Id$
// File:   load_bcidat.cpp
// Author: juergen.mellinger@uni-tuebingen.de
// Date:   Jan 16, 2006
// Description: A Matlab (mex) subroutine that reads BCI2000 .dat files into
//  Matlab workspace variables.
//
//  [ signal, states, parameters ] = load_bcidat( 'filename' )
//
//  loads signal, state, and parameter data from the file whose name is given
//  in the function's argument.
//
//  The 'states' output variable will be a Matlab struct with BCI2000 state
//  names as struct member names, and the number of state value entries matching
//  the first dimension of the 'signal' output variable.
//
//  The 'parameters' output variable will be a Matlab struct with BCI2000
//  parameter names as struct member names.
//  Individual parameters are represented as cell arrays of strings, and may
//  be converted into numeric matrices by Matlab's str2double function.
//
// $Log$
// Revision 1.1  2006/01/17 17:15:47  mellinger
// Initial version.
//
///////////////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "mex.h"
#include "UBCI2000Data.h"
#include "StateRef.h"
#include <sstream>

using namespace std;

typedef signed short int16;
typedef signed int   int32;
typedef float        float32;

template<typename T>
void
ReadSignal( BCI2000DATA& inFile, mxArray* inSignal )
{
  T* data = reinterpret_cast<T*>( mxGetData( inSignal ) );
  int numChannels = inFile.GetNumChannels(),
      numSamples = inFile.GetNumSamples();
  for( int sample = 0; sample < numSamples; ++sample )
    for( int channel = 0; channel < numChannels; ++channel )
      data[ numSamples * channel + sample ] = inFile.ReadValue( channel, sample );
}

void
mexFunction( int nargout, mxArray* varargout[],
             int nargin,  const mxArray* varargin[] )
{
  if( sizeof( int16 ) != 2 || sizeof( int32 ) != 4 || sizeof( float32 ) != 4 )
    mexErrMsgTxt( "Numeric types don't agree with this function's assumptions." );

  if( nargin < 1 )
    mexErrMsgTxt( "No file name given." );
  if( nargin > 1 )
    mexWarnMsgTxt( "Ignoring extra input arguments." );
  char* fileName = mxArrayToString( varargin[ 0 ] );
  if( fileName == NULL )
    mexErrMsgTxt( "Out of memory when reading file name." );

  // Open the file.
  BCI2000DATA file;
  int result = file.Initialize( fileName );
  if( result == BCI2000ERR_FILENOTFOUND )
    result = file.Initialize( ( string( fileName ) + ".dat" ).c_str() );
  switch( result )
  {
    case BCI2000ERR_NOERR:
      break;

    case BCI2000ERR_FILENOTFOUND:
      {
        ostringstream oss;
        oss << "File \"" << fileName << "\" does not exist.";
        mexErrMsgTxt( oss.str().c_str() );
      }
      break;

    default:
      {
        ostringstream oss;
        oss << "Could not open \"" << fileName << "\" as a BCI2000 data file.";
        mexErrMsgTxt( oss.str().c_str() );
      }
  }
  mxFree( fileName );
  __bcierr.clear();

  // Read EEG data into the first output argument.
  mxArray* signal = NULL;
  int dim[] = { file.GetNumSamples(), file.GetNumChannels() };
  switch( file.GetSignalType() )
  {
    case SignalType::int16:
      signal = mxCreateNumericArray( 2, dim, mxINT16_CLASS, mxREAL );
      if( signal != NULL )
        ReadSignal<int16>( file, signal );
      break;

    case SignalType::int32:
      signal = mxCreateNumericArray( 2, dim, mxINT32_CLASS, mxREAL );
      if( signal != NULL )
        ReadSignal<int32>( file, signal );
      break;

    case SignalType::float32:
      signal = mxCreateNumericArray( 2, dim, mxSINGLE_CLASS, mxREAL );
      if( signal != NULL )
        ReadSignal<float32>( file, signal );
      break;

    default:
      mexErrMsgTxt( "Unknown signal type" );
  }
  if( signal == NULL )
    mexErrMsgTxt( "Out of memory when allocating space for the signal variable." );

  __bcierr.clear();
  varargout[ 0 ] = signal;

  // Read state data if appropriate.
  if( nargout > 1 )
  {
    const STATELIST* statelist = file.GetStateListPtr();
    int numStates = statelist->Size();
    char** stateNames = new char*[ numStates ];
    StateRef** stateRefs = new StateRef*[ numStates ];
    int16** stateData = new int16*[ numStates ];
    for( int i = 0; i < numStates; ++i )
    {
      const STATE& s = ( *statelist )[ i ];
      stateNames[ i ] = new char[ strlen( s.GetName() ) + 1 ];
      strcpy( stateNames[ i ], s.GetName() );
      stateRefs[ i ] = new StateRef(
        const_cast<STATEVECTOR*>( file.GetStateVectorPtr() ),
        s.GetLocation(), s.GetLength()
      );
    }
    mxArray* states = mxCreateStructMatrix(
      1, 1, numStates, const_cast<const char**>( stateNames )
    );
    for( int i = 0; i < numStates; ++i )
    {
      mxArray* stateArray = mxCreateNumericMatrix(
        file.GetNumSamples(), 1, mxINT16_CLASS, mxREAL
      );
      if( stateArray == NULL )
        mexErrMsgTxt( "Out of memory when allocating space for state variables." );
      mxSetFieldByNumber( states, 0, i, stateArray );
      stateData[ i ] = reinterpret_cast<int16*>( mxGetData( stateArray ) );
    }
    for( size_t sample = 0; sample < file.GetNumSamples(); ++sample )
    {
      file.ReadStateVector( sample );
      for( int i = 0; i < numStates; ++i )
        *stateData[ i ]++ = ( *stateRefs[ i ] );
    }
    for( int i = 0; i < numStates; ++i )
    {
      delete[] stateNames[ i ];
      delete stateRefs[ i ];
    }
    delete[] stateNames;
    delete[] stateRefs;
    delete[] stateData;

    __bcierr.clear();
    varargout[ 1 ] = states;
  }

  // Read parameters if appropriate.
  if( nargout > 2 )
  {
    const PARAMLIST* paramlist = file.GetParamListPtr();
    int numParams = paramlist->Size();
    char** paramNames = new char*[ numParams ];
    for( int i = 0; i < numParams; ++i )
    {
      const PARAM& param = ( *paramlist )[ i ];
      paramNames[ i ] = new char[ strlen( param.GetName() ) + 1 ];
      strcpy( paramNames[ i ], param.GetName() );
    }
    mxArray* params = mxCreateStructMatrix(
      1, 1, numParams, const_cast<const char**>( paramNames )
    );
    for( int i = 0; i < numParams; ++i )
      delete[] paramNames[ i ];
    delete[] paramNames;
    for( int i = 0; i < numParams; ++i )
    {
      const PARAM& p = ( *paramlist )[ i ];
      mxArray* paramArray = mxCreateCellMatrix( p.GetNumRows(), p.GetNumColumns() );
      if( paramArray == NULL )
        mexErrMsgTxt( "Out of memory when allocating space for state variables." );
      int cell = 0;
      for( size_t col = 0; col < p.GetNumColumns(); ++col )
        for( size_t row = 0; row < p.GetNumRows(); ++row )
          mxSetCell( paramArray, cell++, mxCreateString( p.GetValue( row, col ) ) );
      mxSetFieldByNumber( params, 0, i, paramArray );
    }
    __bcierr.clear();
    varargout[ 2 ] = params;
  }
}
