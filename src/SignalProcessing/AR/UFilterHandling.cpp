////////////////////////////////////////////////////////////////////////////////
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
#include "ARFilter.h"
#include "ClassFilter.h"
#include "NormalFilter.h"
#include "StatFilter.h"

Filter( CalibrationFilter, 2.A );
Filter( SpatialFilter, 2.B );
Filter( ARTemporalFilter, 2.C );
Filter( ClassFilter, 2.D );

Filter( NormalFilter, 2.E1 );

Filter( StatFilter, 2.E2 );

