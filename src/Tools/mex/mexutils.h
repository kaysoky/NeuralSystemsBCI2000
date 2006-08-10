///////////////////////////////////////////////////////////////////////////////
// $Id$
// File:   mexutils.h
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
#ifndef MEXUTILS_H
#define MEXUTILS_H

#include "UParameter.h"
#include "mex.h"

mxArray* ParamlistToStruct( const PARAMLIST& );
mxArray* ParamToCells(      const PARAM& );
mxArray* LabelsToCells(     const PARAM::labelIndexer&, size_t numEntries );

template<typename T>
T*
MexAlloc( int inElements )
{
  // mxCalloc'ed memory will be freed automatically on return from mexFunction().
  return reinterpret_cast<T*>( mxCalloc( inElements, sizeof( T ) ) );
}

#endif // MEXUTILS_H