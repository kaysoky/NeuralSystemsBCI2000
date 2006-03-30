////////////////////////////////////////////////////////////////////////////////
// File:   bci_stubs.cpp
// Date:   Jan 26, 2005
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A file to hold dummy implementations for functions that are
//         unneeded/unwanted when a filter is wrapped into a command line tool.
////////////////////////////////////////////////////////////////////////////////

#include "shared/BCIDirectry.h"
#include "EEGSource/FileWriterBase.h"
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

int
BCIDirectory::ProcPath()
{
  return 0;
}

const char*
BCIDirectory::ProcSubDir()
{
  const char* tempDir = ::getenv( "TEMP" );
  if( tempDir == NULL )
    tempDir = "bci_tool_output";
  return tempDir;
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
FileWriterBase::Preflight( const SignalProperties& Input,
                                 SignalProperties& Output ) const
{
  Output = SignalProperties( 0, 0 );
}


void
FileWriterBase::Initialize2( const SignalProperties& Input,
                             const SignalProperties& Output )
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
FileWriterBase::Write( const GenericSignal& Signal,
                       const STATEVECTOR&   Statevector )
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
