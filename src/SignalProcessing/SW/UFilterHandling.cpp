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
#include "FFTFilter.h"
#include "SWFilter.h"
#include "AverageDisplay.h"
#include "SetBaseline.h"
#include "FBArteCorrection.h"
#include "NormalFilter.h"

Filter( CalibrationFilter, 2.A );
Filter( SpatialFilter, 2.B );
Filter( FFTFilter, 2.B1 );
Filter( AverageDisplay, 2.B2 );
Filter( TSWFilter, 2.C );
Filter( TSetBaseline, 2.D1 );
Filter( TFBArteCorrection, 2.D2 );
Filter( NormalFilter, 2.E );

