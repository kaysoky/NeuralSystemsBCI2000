////////////////////////////////////////////////////////////////////////////////
//
// File: UGenericFilter.cpp
//
// Description: Definitions for the GenericFilter interface
//   which all BCI2000 filters are supposed to implement.
//
// Changes: Oct 21, 2002, juergen.mellinger@uni-tuebingen.de
//          - Made GenericFilter a true base class, and a purely abstract one.
//
////////////////////////////////////////////////////////////////////////////////
#include "UGenericFilter.h"

GenericFilter::filterSet GenericFilter::filters;

