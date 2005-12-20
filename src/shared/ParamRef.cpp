//////////////////////////////////////////////////////////////////////
// $Id$
//
// File: ParamRef.cpp
//
// Date: Oct 31, 2005
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// $Log$
// Revision 1.2  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
//
// Description: A class that holds references to parameters or
//         parameter values, and allows for convenient automatic type
//         conversions.
//
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ParamRef.h"

PARAM ParamRef::sNullParam;

ParamRef
ParamRef::operator()( size_t row, const std::string& col_label ) const
{
  PARAM* subParam = operator->();
  size_t col_idx = subParam->ColumnLabels()[ col_label ];
  return ParamRef( subParam, row, col_idx );
}

ParamRef
ParamRef::operator()( const std::string& row_label, size_t col ) const
{
  PARAM* subParam = operator->();
  size_t row_idx = subParam->RowLabels()[ row_label ];
  return ParamRef( subParam, row_idx, col );
}

ParamRef
ParamRef::operator()( const std::string& row_label, const std::string& col_label ) const
{
  PARAM* subParam = operator->();
  size_t row_idx = subParam->RowLabels()[ row_label ],
         col_idx = subParam->ColumnLabels()[ col_label ];
  return ParamRef( subParam, row_idx, col_idx );
}



