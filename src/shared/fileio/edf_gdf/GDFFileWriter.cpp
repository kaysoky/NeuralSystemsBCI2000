////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that stores data into a EDF or GDF data file.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GDFFileWriter.h"

// File writer filters must have a position string greater than
// that of the DataIOFilter.
RegisterFilter( GDFFileWriter, 1 );

