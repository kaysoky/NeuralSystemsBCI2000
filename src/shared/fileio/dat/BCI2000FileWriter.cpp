////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A FileWriter filter that stores data into a BCI2000 dat file.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCI2000FileWriter.h"

#include <string>

using namespace std;

// File writer filters must have a position string greater than
// that of the DataIOFilter.
RegisterFilter( BCI2000FileWriter, 1 );

void
BCI2000FileWriter::Publish() const
{
  FileWriterBase::Publish();

  BEGIN_PARAMETER_DEFINITIONS
    "Storage:Documentation string StorageTime= % % % % "
      "// time of beginning of data storage",
  END_PARAMETER_DEFINITIONS
}

void
BCI2000FileWriter::Preflight( const SignalProperties& Input,
                                    SignalProperties& Output ) const
{
  FileWriterBase::Preflight( Input, Output );
  
  if( !string( Parameter( "StorageTime" ) ).empty() )
    bciout << "The StorageTime parameter will be overwritten with the"
           << " recording's actual date and time"
           << endl;
}

void
BCI2000FileWriter::StartRun()
{
  time_t now = ::time( NULL );
  const char* dateTime = ::ctime( &now );
  if( dateTime != NULL )
  {
    std::string strDateTime( dateTime, strlen( dateTime ) - 1 );
    Parameter( "StorageTime" ) = strDateTime;
  }
  FileWriterBase::StartRun();
}

