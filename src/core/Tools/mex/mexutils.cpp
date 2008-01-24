///////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Utility functions for Matlab mex files that deal with BCI2000
//         files.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////
#include "mexutils.h"
#include "mex.h"
#include <sstream>

using namespace std;

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
    mexErrMsgTxt( "Out of memory when allocating space for a parameter." );

  struct
  {
    const char*    name;
    mxArray* value;
  } fields[] =
  {
    { "Section",      mxCreateString( p.Section().c_str() ) },
    { "Type",         mxCreateString( p.Type().c_str() ) },
    { "DefaultValue", mxCreateString( p.DefaultValue().c_str() ) },
    { "LowRange",     mxCreateString( p.LowRange().c_str() ) },
    { "HighRange",    mxCreateString( p.HighRange().c_str() ) },
    { "Comment",      mxCreateString( p.Comment().c_str() ) },
    { "Value",        ValuesToCells( p ) },
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
    mexErrMsgTxt( "Out of memory when allocating space for parameter values." );
  int cell = 0;
  for( int col = 0; col < p.NumColumns(); ++col )
    for( int row = 0; row < p.NumRows(); ++row, ++cell )
      if( p.Value().Kind() != Param::ParamValue::Single )
        mxSetCell( paramArray, cell, ValuesToCells( *p.Value().ToParam() ) );
      else
        mxSetCell( paramArray, cell, mxCreateString( p.Value( row, col ).c_str() ) );

  return paramArray;
}

mxArray*
LabelsToCells( const LabelIndex& labels, size_t numEntries )
{
  mxArray* labelArray = mxCreateCellMatrix( numEntries, 1 );
  if( labelArray == NULL )
    mexErrMsgTxt( "Out of memory when allocating space for parameter labels." );
  for( size_t idx = 0; idx < numEntries; ++idx )
    mxSetCell( labelArray, idx, mxCreateString( labels[ idx ].c_str() ) );

  return labelArray;
}


void
StructToParamlist( const mxArray* inStruct, ParamList& outParamlist )
{
  if( !mxIsStruct( inStruct ) )
    mexErrMsgTxt( "Input argument is not a Matlab struct." );

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
    mexErrMsgTxt( "Could not access \"Value\" field." );
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
          mexErrMsgTxt( "Parameter values must be strings or cell arrays of strings." );
      }
    }
}

void
CellsToLabels( const mxArray* inCells, LabelIndex& ioIndexer )
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


