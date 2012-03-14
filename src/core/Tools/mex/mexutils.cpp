///////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Utility functions for Matlab mex files that deal with BCI2000
//         files.
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
///////////////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "mexutils.h"
#include "Version.h"
#include "VersionInfo.h"
#include "ArithmeticExpression.h"
#include "BCIError.h"
#include "BCIException.h"
#include "mex.h"
#include <sstream>
#include <cmath>
#include <cstring>

using namespace std;

void
TypeCheck()
{
  bool ok = 
  (
    sizeof( int16 ) == 2 && sizeof( int32 ) == 4 && sizeof( float32 ) == 4
    && sizeof( uint8 ) == 1 && sizeof( uint16 ) == 2 && sizeof( uint32 ) == 4
    && sizeof( uint64 ) == 8
  );
  if( !ok )
    throw bciexception_( "Numeric types don't agree with this function's assumptions." );
}

bool
PrintVersion( const char* inSourceFile, int inNargin, const mxArray** inVarargin )
{
  bool argFound = false;
  for( int i = 0; i < inNargin && !argFound; ++i )
  {
    if( mxCHAR_CLASS == mxGetClassID( inVarargin[ i ] ) )
    {
      char* arg = mxArrayToString( inVarargin[ i ] );
      if( 0 == stricmp( arg, "--version" ) || 0 == stricmp( arg, "-v" ) )
        argFound = true;
      mxFree( arg );
    }
  }
  if( argFound )
  {
    string mexName( inSourceFile );
    mexName = mexName.substr( 0, mexName.rfind( "." ) );
    ostringstream oss;
    oss << mexName << " BCI2000 mex file:\n";
    VersionInfo::Current.WriteToStream( oss, true );
    oss << BCI2000_COPYRIGHT << '\n';
    mexPrintf( "%s", oss.str().c_str() );
  }
  return argFound;
}

mxArray*
ParamlistToStruct( const ParamList& inParamlist )
{
  int numParams = inParamlist.Size();
  char** paramNames = MexAlloc<char*>( numParams );
  for( int i = 0; i < numParams; ++i )
  {
    const Param& param = inParamlist[ i ];
    paramNames[ i ] = MexAlloc<char>( param.Name().length() + 1 );
    strcpy( paramNames[ i ], param.Name().c_str() );
  }
  mxArray* params = mxCreateStructMatrix(
    1, 1, numParams, const_cast<const char**>( paramNames )
  );
  for( int i = 0; i < numParams; ++i )
    mxSetFieldByNumber( params, 0, i, ParamToStruct( inParamlist[ i ] ) );
  return params;
}

mxArray*
ParamToStruct( const Param& p )
{
  mxArray* param = mxCreateStructMatrix( 1, 1, 0, NULL );
  if( param == NULL )
    throw bciexception_( "Out of memory when allocating space for a parameter." );

  struct
  {
    const char* name;
    mxArray*    value;
  } fields[] =
  {
    { "Section",      mxCreateString( p.Section().c_str() ) },
    { "Type",         mxCreateString( p.Type().c_str() ) },
    { "DefaultValue", mxCreateString( p.DefaultValue().c_str() ) },
    { "LowRange",     mxCreateString( p.LowRange().c_str() ) },
    { "HighRange",    mxCreateString( p.HighRange().c_str() ) },
    { "Comment",      mxCreateString( p.Comment().c_str() ) },
    { "Value",        ValuesToCells( p ) },
    { "NumericValue", ValuesToNumbers( p ) },
    { "RowLabels",    p.RowLabels().IsTrivial()
                       ? NULL
                       : LabelsToCells( p.RowLabels(), p.NumRows() ) },
    { "ColumnLabels", p.ColumnLabels().IsTrivial()
                       ? NULL
                       : LabelsToCells( p.ColumnLabels(), p.NumColumns() ) },
  };
  const size_t numFields = sizeof( fields ) / sizeof( *fields );
  for( size_t j = 0; j < numFields; ++j )
    if( fields[ j ].value != NULL )
    {
      int idx = mxAddField( param, fields[ j ].name );
      mxSetFieldByNumber( param, 0, idx, fields[ j ].value );
    }
  return param;
}

mxArray*
ValuesToCells( const Param& p )
{
  mxArray* paramArray = mxCreateCellMatrix( p.NumRows(), p.NumColumns() );
  if( paramArray == NULL )
    throw bciexception_( "Out of memory when allocating space for parameter values." );
  int cell = 0;
  for( int col = 0; col < p.NumColumns(); ++col )
    for( int row = 0; row < p.NumRows(); ++row, ++cell )
      if( p.Value( row, col ).Kind() != Param::ParamValue::Single )
        mxSetCell( paramArray, cell, ValuesToCells( *p.Value( row, col ).ToParam() ) );
      else
        mxSetCell( paramArray, cell, mxCreateString( p.Value( row, col ).c_str() ) );

  return paramArray;
}

mxArray*
ValuesToNumbers( const Param& p )
{
  // Don't create a numeric array for nested matrices.
  bool isNested = false;
  for( int col = 0; !isNested && col < p.NumColumns(); ++col )
    for( int row = 0; !isNested && row < p.NumRows(); ++row )
      if( p.Value().Kind() != Param::ParamValue::Single )
        isNested = true;
  if( isNested )
    return NULL;

  mxArray* paramArray = mxCreateDoubleMatrix( p.NumRows(), p.NumColumns(), mxREAL );
  if( paramArray == NULL )
    throw bciexception_( "Out of memory when allocating space for parameter values." );
  double* pMatrix = mxGetPr( paramArray );

  int cell = 0;
  for( int col = 0; col < p.NumColumns(); ++col )
    for( int row = 0; row < p.NumRows(); ++row, ++cell )
    {
      // Remove trailing characters to account for units.
      string value = p.Value( row, col );
      int i = value.length() - 1;
      while( i > 0 && isalpha( value[ i ] ) )
        --i;
      value = value.substr( 0, i + 1 );
      // Evaluate the remaining string as an expression if possible.
      if( value.empty() )
        pMatrix[ cell ] = mxGetNaN();
      else
      {
        ArithmeticExpression expr( value );
        if( expr.IsValid() )
          pMatrix[ cell ] = expr.Evaluate();
        else
          pMatrix[ cell ] = mxGetNaN();
      }
    }

  return paramArray;
}

mxArray*
LabelsToCells( const LabelIndex& labels, size_t numEntries )
{
  mxArray* labelArray = mxCreateCellMatrix( numEntries, 1 );
  if( labelArray == NULL )
    throw bciexception_( "Out of memory when allocating space for parameter labels." );
  for( size_t idx = 0; idx < numEntries; ++idx )
    mxSetCell( labelArray, idx, mxCreateString( labels[ idx ].c_str() ) );

  return labelArray;
}


void
StructToParamlist( const mxArray* inStruct, ParamList& outParamlist )
{
  if( !mxIsStruct( inStruct ) )
    throw bciexception_( "Input argument is not a Matlab struct." );

  outParamlist.Clear();
  size_t numParams = mxGetNumberOfFields( inStruct );
  for( size_t i = 0; i < numParams; ++i )
  {
    Param p = StructToParam(
        mxGetFieldByNumber( inStruct, 0, i ),
        mxGetFieldNameByNumber( inStruct, i )
    );
    outParamlist.Add( p );
  }
}

Param
StructToParam( const mxArray* inStruct, const char* inName )
{
  char* section = GetStringField( inStruct, "Section" ),
      * type = GetStringField( inStruct, "Type" ),
      * defaultValue = GetStringField( inStruct, "DefaultValue" ),
      * lowRange = GetStringField( inStruct, "LowRange" ),
      * highRange = GetStringField( inStruct, "HighRange" ),
      * comment = GetStringField( inStruct, "Comment" );
  Param p( inName, section, type, "" /* Value */,
           defaultValue, lowRange, highRange, comment );
  mxFree( section );
  mxFree( type );
  mxFree( defaultValue );
  mxFree( lowRange );
  mxFree( highRange );
  mxFree( comment );

  mxArray* values = mxGetField( inStruct, 0, "Value" ),
         * rowLabels = mxGetField( inStruct, 0, "RowLabels" ),
         * colLabels = mxGetField( inStruct, 0, "ColumnLabels" );
  if( values == NULL )
    throw bciexception_( "Could not access \"Value\" field." );
  CellsToValues( values, p );
  if( rowLabels != NULL )
    CellsToLabels( rowLabels, p.RowLabels() );
  if( colLabels != NULL )
    CellsToLabels( colLabels, p.ColumnLabels() );

  return p;
}

void
CellsToValues( const mxArray* inCells, Param& ioParam )
{
  const mwSize* dim = mxGetDimensions( inCells );
  mwSize numRows = dim[ 0 ],
         numCols = dim[ 1 ];
  if( ( ioParam.Type().find( "list" ) != string::npos ) && ( numCols == 1 ) )
    ioParam.SetNumValues( numRows );
  else if( numRows * numCols > 1 )
    ioParam.SetDimensions( numRows, numCols );
  for( mwIndex row = 0; row < numRows; ++row )
    for( mwIndex col = 0; col < numCols; ++col )
    {
      mwIndex idx[] = { row, col };
      const mxArray* cell = mxGetCell( inCells, mxCalcSingleSubscript( inCells, 2, idx ) );
      switch( cell ? mxGetClassID( cell ) : mxUNKNOWN_CLASS )
      {
        case mxCHAR_CLASS:
          ioParam.Value( row, col ) = mxArrayToString( cell );
          break;

        case mxCELL_CLASS:
          {
            Param p;
            CellsToValues( cell, p );
            ioParam.Value( row, col ) = p;
          }
          break;

        default:
          throw bciexception_( "Parameter values must be strings or cell arrays of strings." );
      }
    }
}

void
CellsToLabels( const mxArray* inCells, LabelIndex& ioIndexer )
{
  mwSize numLabels = mxGetNumberOfElements( inCells );
  if( numLabels > ioIndexer.Size() )
    ioIndexer.Resize( numLabels );
  for( int i = 0; i < numLabels; ++i )
  {
    const mxArray* cell = mxGetCell( inCells, i );
    switch( mxGetClassID( cell ) )
    {
      case mxCHAR_CLASS:
        ioIndexer[ i ] = mxArrayToString( cell );
        break;

      default:
        throw bciexception_( "Parameter labels must be strings." );
    }
  }
}

char*
GetStringField( const mxArray* inStruct, const char* inName )
{
  mxArray* field = mxGetField( inStruct, 0, inName );
  if( field == NULL || !mxIsChar( field ) )
    throw bciexception_( "Could not access string field \"" << inName << "\"." );
  return mxArrayToString( field );
}

#undef mexFunction
void
mexFunction( int nargout, mxArray* varargout[],
             int nargin,  const mxArray* varargin[] )
{
  bcierr__.Reset();
  try
  {
    TypeCheck();
    BCIMexFunction( nargout, varargout, nargin, varargin );
  }
  catch( const BCIException& e )
  {
#undef mexErrMsgTxt
    ::mexErrMsgTxt( e.what() );
  }
}