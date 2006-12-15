///////////////////////////////////////////////////////////////////////////////
// $Id$
// File:   mexutils.cpp
// Author: juergen.mellinger@uni-tuebingen.de
// Date:   Aug 10, 2006
// Description: Utility functions for Matlab mex files that deal with BCI2000
//         files.
//
// $Log$
// Revision 1.1  2006/08/10 15:38:37  mellinger
// Initial version.
//
//
///////////////////////////////////////////////////////////////////////////////
#include "mexutils.h"
#include "mex.h"
#include <sstream>

using namespace std;

mxArray*
ParamlistToStruct( const PARAMLIST& inParamlist )
{
  int numParams = inParamlist.Size();
  char** paramNames = MexAlloc<char*>( numParams );
  for( int i = 0; i < numParams; ++i )
  {
    const PARAM& param = inParamlist[ i ];
    paramNames[ i ] = MexAlloc<char>( strlen( param.GetName() ) + 1 );
    strcpy( paramNames[ i ], param.GetName() );
  }
  mxArray* params = mxCreateStructMatrix(
    1, 1, numParams, const_cast<const char**>( paramNames )
  );
  for( int i = 0; i < numParams; ++i )
    mxSetFieldByNumber( params, 0, i, ParamToStruct( inParamlist[ i ] ) );
  return params;
}

mxArray*
ParamToStruct( const PARAM& p )
{
  mxArray* param = mxCreateStructMatrix( 1, 1, 0, NULL );
  if( param == NULL )
    mexErrMsgTxt( "Out of memory when allocating space for a parameter." );

  struct
  {
    const char*    name;
    mxArray* value;
  } fields[] =
  {
    { "Section",      mxCreateString( p.GetSection() ) },
    { "Type",         mxCreateString( p.GetType() ) },
    { "DefaultValue", mxCreateString( p.GetDefaultValue() ) },
    { "LowRange",     mxCreateString( p.GetLowRange() ) },
    { "HighRange",    mxCreateString( p.GetHighRange() ) },
    { "Comment",      mxCreateString( p.GetComment() ) },
    { "Value",        ValuesToCells( p ) },
    { "RowLabels",    p.RowLabels().IsTrivial()
                       ? NULL
                       : LabelsToCells( p.RowLabels(), p.GetNumRows() ) },
    { "ColumnLabels", p.ColumnLabels().IsTrivial()
                       ? NULL
                       : LabelsToCells( p.ColumnLabels(), p.GetNumColumns() ) },
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
ValuesToCells( const PARAM& p )
{
  mxArray* paramArray = mxCreateCellMatrix( p.GetNumRows(), p.GetNumColumns() );
  if( paramArray == NULL )
    mexErrMsgTxt( "Out of memory when allocating space for parameter values." );
  int cell = 0;
  for( size_t col = 0; col < p.GetNumColumns(); ++col )
    for( size_t row = 0; row < p.GetNumRows(); ++row, ++cell )
      if( p.Value().Kind() != PARAM::paramValue::Single )
        mxSetCell( paramArray, cell, ValuesToCells( *p.Value().ToParam() ) );
      else
        mxSetCell( paramArray, cell, mxCreateString( p.Value( row, col ) ) );

  return paramArray;
}

mxArray*
LabelsToCells( const PARAM::labelIndexer& labels, size_t numEntries )
{
  mxArray* labelArray = mxCreateCellMatrix( numEntries, 1 );
  if( labelArray == NULL )
    mexErrMsgTxt( "Out of memory when allocating space for parameter labels." );
  for( size_t idx = 0; idx < numEntries; ++idx )
    mxSetCell( labelArray, idx, mxCreateString( labels[ idx ].c_str() ) );

  return labelArray;
}


void
StructToParamlist( const mxArray* inStruct, PARAMLIST& outParamlist )
{
  if( !mxIsStruct( inStruct ) )
    mexErrMsgTxt( "Input argument is not a Matlab struct." );

  outParamlist.Clear();
  size_t numParams = mxGetNumberOfFields( inStruct );
  for( size_t i = 0; i < numParams; ++i )
  {
    PARAM p = StructToParam(
        mxGetFieldByNumber( inStruct, 0, i ),
        mxGetFieldNameByNumber( inStruct, i )
    );
    outParamlist.Add( p );
  }
}

PARAM
StructToParam( const mxArray* inStruct, const char* inName )
{
  char* section = GetStringField( inStruct, "Section" ),
      * type = GetStringField( inStruct, "Type" ),
      * defaultValue = GetStringField( inStruct, "DefaultValue" ),
      * lowRange = GetStringField( inStruct, "LowRange" ),
      * highRange = GetStringField( inStruct, "HighRange" ),
      * comment = GetStringField( inStruct, "Comment" );
  PARAM p( inName, section, type, "" /* Value */,
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
    mexErrMsgTxt( "Could not access \"Value\" field." );
  CellsToValues( values, p );
  if( rowLabels != NULL )
    CellsToLabels( rowLabels, p.RowLabels() );
  if( colLabels != NULL )
    CellsToLabels( colLabels, p.ColumnLabels() );

  return p;
}

void
CellsToValues( const mxArray* inCells, PARAM& ioParam )
{
  const int* dim = mxGetDimensions( inCells );
  int numRows = dim[ 0 ],
      numCols = dim[ 1 ];
  if( numRows * numCols > 1 )
    ioParam.SetDimensions( dim[ 0 ], dim[ 1 ] );
  for( int row = 0; row < dim[ 0 ]; ++row )
    for( int col = 0; col < dim[ 1 ]; ++col )
    {
      int idx[] = { row, col };
      const mxArray* cell = mxGetCell( inCells, mxCalcSingleSubscript( inCells, 2, idx ) );
      switch( mxGetClassID( cell ) )
      {
        case mxCHAR_CLASS:
          ioParam.Value( row, col ) = mxArrayToString( cell );
          break;

        case mxCELL_CLASS:
          {
            PARAM p;
            CellsToValues( cell, p );
            ioParam.Value( row, col ) = p;
          }
          break;

        default:
          mexErrMsgTxt( "Parameter values must be strings or cell arrays of strings." );
      }
    }
}

void
CellsToLabels( const mxArray* inCells, PARAM::labelIndexer& ioIndexer )
{
  int numLabels = mxGetNumberOfElements( inCells );
  for( int i = 0; i < numLabels; ++i )
  {
    const mxArray* cell = mxGetCell( inCells, i );
    switch( mxGetClassID( cell ) )
    {
      case mxCHAR_CLASS:
        ioIndexer[ i ] = mxArrayToString( cell );
        break;

      default:
        mexErrMsgTxt( "Parameter labels must be strings." );
    }
  }
}

char*
GetStringField( const mxArray* inStruct, const char* inName )
{
  mxArray* field = mxGetField( inStruct, 0, inName );
  if( field == NULL || !mxIsChar( field ) )
  {
    ostringstream oss;
    oss << "Could not access string field \"" << inName << "\".";
    mexErrMsgTxt( oss.str().c_str() );
  }
  return mxArrayToString( field );
}


