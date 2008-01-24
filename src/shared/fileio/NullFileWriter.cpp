////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A file writer class that does _not_ write out any data.
//   Useful when no data file output is desired.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "NullFileWriter.h"

using namespace std;

// File writer filters must have a position string greater than
// that of the DataIOFilter.
RegisterFilter( NullFileWriter, 1 );


NullFileWriter::NullFileWriter()
{
}


NullFileWriter::~NullFileWriter()
{
}

void
NullFileWriter::Publish() const
{
}


void
NullFileWriter::Preflight( const SignalProperties&,
                                 SignalProperties& Output ) const
{
  Output = SignalProperties( 0, 0 );
}


void
NullFileWriter::Initialize( const SignalProperties& /*Input*/,
                            const SignalProperties& /*Output*/ )
{
}


void
NullFileWriter::Write( const GenericSignal& /*inSignal*/,
                       const StateVector&   /*inStatevector*/ )
{
}

