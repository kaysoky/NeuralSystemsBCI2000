////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that stores data into a EDF or EDF data file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "EDFFileWriter.h"

// File writer filters must have a position string greater than
// that of the DataIOFilter.
RegisterFilter( EDFFileWriter, 1 );
