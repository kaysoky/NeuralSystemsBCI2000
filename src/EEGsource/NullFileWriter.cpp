////////////////////////////////////////////////////////////////////////////////
//
// File:NullFileWriter.cpp
//
// Date: Sept 20, 2005
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A dummy file writer.
//
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
NullFileWriter::Initialize2( const SignalProperties& Input,
                             const SignalProperties& Output )
{
}


void
NullFileWriter::Write( const GenericSignal& inSignal,
                       const STATEVECTOR& inStatevector )
{
}

