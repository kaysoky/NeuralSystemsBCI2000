////////////////////////////////////////////////////////////////////////////////
//
// This file defines which filters will be used, and the sequence in which they
// are applied.
// Each Filter() entry consists of the name of the filter and a string which,
// by lexical comparison, defines the relative position of the filter in the
// sequence.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "CalibrationFilter.h"
#include "SpatialFilter.h"
#include "FIRFilter.h"
#include "ClassFilter.h"
#include "NormalFilter.h"
#include "StatFilter.h"

Filter( CalibrationFilter, 2.A );
Filter( SpatialFilter, 2.B );
Filter( FIRFilter, 2.C );
Filter( ClassFilter, 2.D );
Filter( StatFilter, 2.E1 );
Filter( NormalFilter, 2.E2 );

