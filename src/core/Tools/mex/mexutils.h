///////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Utility functions for Matlab mex files that deal with BCI2000
//         files.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////
#ifndef MEXUTILS_H
#define MEXUTILS_H

#include "Param.h"
#include "ParamList.h"
#include "LabelIndex.h"
#include "mex.h"

mxArray* ParamlistToStruct( const ParamList& );
mxArray* ParamToStruct(     const Param& );
mxArray* ValuesToCells(     const Param& );
mxArray* LabelsToCells(     const LabelIndex&, size_t numEntries );

void  StructToParamlist( const mxArray*, ParamList& );
Param StructToParam(     const mxArray*, const char* name );
void  CellsToValues(     const mxArray*, Param& );
void  CellsToLabels(     const mxArray*, LabelIndex& );

char* GetStringField( const mxArray*, const char* name );

template<typename T>
T*
MexAlloc( int inElements )
{
  // mxCalloc'ed memory will be freed automatically on return from mexFunction().
  return reinterpret_cast<T*>( mxCalloc( inElements, sizeof( T ) ) );
}

#endif // MEXUTILS_H
