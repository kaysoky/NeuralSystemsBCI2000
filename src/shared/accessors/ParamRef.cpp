//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that holds references to parameters or
//         parameter values, and allows for convenient automatic type
//         conversions.
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
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ParamRef.h"
#include <cstdlib>

using namespace std;

Param  ParamRef::sNullParam;
string ParamRef::sNullString;

ParamRef::operator double() const
{
  double result = 0.0;
  if( mpParam )
  {
    const Param* p = mpParam; // const access
    const std::string& val = p->Value( index( mIdx1 ), index( mIdx2 ) );
    result = std::atof( val.c_str() );
    if( result == 0.0 )
    {
      uint64_t n = 0;
      if( std::istringstream( val ) >> std::hex >> n )
        result = static_cast<double>( n );
    }
  }
  return result;
}

ParamRef
ParamRef::operator()( size_t row, const string& col_label ) const
{
  const Param* subParam = operator->();
  size_t col_idx = subParam->ColumnLabels()[ col_label ];
  return ParamRef( const_cast<Param*>( subParam ), row, col_idx );
}

ParamRef
ParamRef::operator()( const string& row_label, size_t col ) const
{
  const Param* subParam = operator->();
  size_t row_idx = subParam->RowLabels()[ row_label ];
  return ParamRef( const_cast<Param*>( subParam ), row_idx, col );
}

ParamRef
ParamRef::operator()( const string& row_label, const string& col_label ) const
{
  const Param* subParam = operator->();
  size_t row_idx = subParam->RowLabels()[ row_label ],
         col_idx = subParam->ColumnLabels()[ col_label ];
  return ParamRef( const_cast<Param*>( subParam ), row_idx, col_idx );
}




