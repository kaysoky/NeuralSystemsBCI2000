////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A virtual class interface for data output filters.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GenericFileWriter.h"

void
GenericFileWriter::CallPublish()
{
  Environment::ErrorContext( "Publish", this );
  this->Publish();
  Environment::ErrorContext( "" );
}

void
GenericFileWriter::CallWrite( const GenericSignal& inData, const StateVector& inStatevector )
{
  Environment::ErrorContext( "Write", this );
  this->Write( inData, inStatevector );
  Environment::ErrorContext( "" );
}

