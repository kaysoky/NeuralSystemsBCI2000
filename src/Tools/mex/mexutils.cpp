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
  {
    const PARAM& p = inParamlist[ i ];
    mxArray* curParam = mxCreateStructMatrix( 1, 1, 0, NULL );
    if( curParam == NULL )
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
      { "Value",        ParamToCells( p ) },
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
        int idx = mxAddField( curParam, fields[ j ].name );
        mxSetFieldByNumber( curParam, 0, idx, fields[ j ].value );
      }

    mxSetFieldByNumber( params, 0, i, curParam );
  }
  return params;
}

mxArray*
ParamToCells( const PARAM& p )
{
  mxArray* paramArray = mxCreateCellMatrix( p.GetNumRows(), p.GetNumColumns() );
  if( paramArray == NULL )
    mexErrMsgTxt( "Out of memory when allocating space for parameter values." );
  int cell = 0;
  for( size_t col = 0; col < p.GetNumColumns(); ++col )
    for( size_t row = 0; row < p.GetNumRows(); ++row, ++cell )
      if( p.Value().Kind() != PARAM::paramValue::Single )
        mxSetCell( paramArray, cell, ParamToCells( *p.Value().ToParam() ) );
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

