////////////////////////////////////////////////////////////////////////////////
// $Id: PipeDefinition.cpp 1454 2007-07-27 15:33:10Z mellinger $
// Description: This file defines which filters will be used, and the sequence
//   in which they are applied.
//   Each Filter() entry consists of the name of the filter and a string which,
//   by lexical comparison, defines the relative position of the filter in the
//   sequence.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SigfriedARFilter.h"
#include "LinearClassifier.h"
#include "Normalizer.h"

Filter( SigfriedARFilter, 2.C );
Filter( LinearClassifier, 2.D );
Filter( Normalizer, 2.E );


