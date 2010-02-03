////////////////////////////////////////////////////////////////////////////////
// $Id: PipeDefinition.cpp,v 1.2 2008/06/19 08:55:24 jurmel Exp $
// Description: This file defines which filters will be used, and the sequence
//   in which they are applied.
//   Each Filter() entry consists of the name of the filter and a string which,
//   by lexical comparison, defines the relative position of the filter in the
//   sequence.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "FieldTripBufferFilter.h"
#include "ExpressionFilter.h"

Filter( FieldTripBufferFilter, 2.B );
Filter( ExpressionFilter, 2.C );
