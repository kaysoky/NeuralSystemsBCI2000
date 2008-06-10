////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A FileWriter filter that stores data into a BCI2000 dat file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCI2000FileWriter.h"

// File writer filters must have a position string greater than
// that of the DataIOFilter.
RegisterFilter( BCI2000FileWriter, 1 );

