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

#include "DummyFilter.h"

Filter( DummyFilter, 2.Z );

