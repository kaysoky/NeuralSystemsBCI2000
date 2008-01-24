//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that holds references to parameters or
//         parameter values, and allows for convenient automatic type
//         conversions.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ParamRef.h"

using namespace std;

Param  ParamRef::sNullParam;
string ParamRef::sNullString;

ParamRef::operator double() const
{
  double result = 0.0;
  if( mpParam )
  {
    const std::string& val = mpParam->Value( index( mIdx1 ), index( mIdx2 ) );
    result = std::atof( val.c_str() );
    if( result == 0.0 )
    {
      long long n = 0;
      if( std::istringstream( val ) >> std::hex >> n )
        result = n;
    }
  }
  return result;
}

ParamRef
ParamRef::operator()( size_t row, const string& col_label ) const
{
  Param* subParam = operator->();
  size_t col_idx = subParam->ColumnLabels()[ col_label ];
  return ParamRef( subParam, row, col_idx );
}

ParamRef
ParamRef::operator()( const string& row_label, size_t col ) const
{
  Param* subParam = operator->();
  size_t row_idx = subParam->RowLabels()[ row_label ];
  return ParamRef( subParam, row_idx, col );
}

ParamRef
ParamRef::operator()( const string& row_label, const string& col_label ) const
{
  Param* subParam = operator->();
  size_t row_idx = subParam->RowLabels()[ row_label ],
         col_idx = subParam->ColumnLabels()[ col_label ];
  return ParamRef( subParam, row_idx, col_idx );
}





