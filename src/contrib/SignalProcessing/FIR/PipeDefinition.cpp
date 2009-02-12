////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: This file defines which filters will be used, and the sequence
//   in which they are applied.
//   Each Filter() entry consists of the name of the filter and a string which,
//   by lexical comparison, defines the relative position of the filter in the
//   sequence.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SpatialFilter.h"
#include "FIRFilter.h"
#include "LinearClassifier.h"
#include "LPFilter.h"
#include "Normalizer.h"

Filter( SpatialFilter, 2.B );
Filter( FIRFilter, 2.C );
Filter( LinearClassifier, 2.D );
Filter( LPFilter, 2.E1 );
Filter( Normalizer, 2.E2 );

