////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A file to hold dummy implementations for functions that are
//         unneeded/unwanted when a filter is wrapped into a command line tool.
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "BCIDirectory.h"
#include "FileWriterBase.h"
#include <iostream>
#include <cstdlib>

using namespace std;

// BCIDirectory

BCIDirectory::BCIDirectory()
: mSessionNumber( none ),
  mDesiredRunNumber( none ),
  mActualRunNumber( none ),
  mFileExtension( "" )
{
}

BCIDirectory&
BCIDirectory::UpdateRunNumber()
{
  return *this;
}

// FileWriterBase

FileWriterBase::FileWriterBase()
{
}


FileWriterBase::~FileWriterBase()
{
}


void
FileWriterBase::Publish() const
{
}


void
FileWriterBase::Preflight( const SignalProperties&,
                                 SignalProperties& Output ) const
{
  Output = SignalProperties( 0, 0 );
}


void
FileWriterBase::Initialize( const SignalProperties&,
                            const SignalProperties& )
{
  cout.clear();
}


void
FileWriterBase::StartRun()
{
  cout.clear();
}


void
FileWriterBase::StopRun()
{
  cout.clear();
}


void
FileWriterBase::Write( const GenericSignal&,
                       const StateVector& )
{
  if( !cout )
    bcierr << "Error writing to stdout" << endl;
  State( "Recording" ) = ( cout ? 1 : 0 );
}


std::ostream&
FileWriterBase::OutputStream()
{
  return cout;
}
