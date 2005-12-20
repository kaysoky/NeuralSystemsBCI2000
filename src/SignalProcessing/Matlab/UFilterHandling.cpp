////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// This file defines which filters will be used, and the sequence in which they
// are applied.
// Each Filter() entry consists of the name of the filter and a string which,
// by lexical comparison, defines the relative position of the filter in the
// sequence.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "CalibrationFilter.h"
#include "SpatialFilter.h"
#include "MatlabFilter.h"

Filter( CalibrationFilter, 2.A );
Filter( SpatialFilter, 2.B );
Filter( MatlabFilter, 2.C );
