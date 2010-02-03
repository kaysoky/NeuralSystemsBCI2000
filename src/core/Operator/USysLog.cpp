/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop

#include "USysLog.h"

// **************************************************************************
// Function:   SYSLOG
// Purpose:    The constructor for the SYSLOG class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SYSLOG::SYSLOG()
: mpForm( new TForm( ( TComponent* )NULL ) ),
  mpLog( new TRichEdit( mpForm ) ),
  mpCritsec( new TCriticalSection ),
  mDontClose( false )
{
  //for( int i = 0; i < numLogEntryModes; ++i )
  //  mTextAttributes[ i ] = new TTextAttributes;
  mpForm->Caption = "System Log";
  mpForm->Width = 600;
  mpForm->Height = 250;
  // mpForm->BorderIcons >> biSystemMenu;
  // mpForm->BorderStyle = bsSizeable;

  mpLog->Visible = false;
  mpLog->Parent = mpForm;
  mpLog->Left = 0;
  mpLog->Top = 0;
  mpLog->Width = mpForm->ClientWidth;
  mpLog->Height = mpForm->ClientHeight;
  mpLog->Anchors << akLeft << akTop << akRight << akBottom;
  mpLog->ScrollBars = ssVertical;
  mpLog->Visible = true;
}


// **************************************************************************
// Function:   SYSLOG
// Purpose:    The destructor for the SYSLOG class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SYSLOG::~SYSLOG()
{
  delete mpCritsec;
  delete mpForm;
}

// The user must close the syslog manually if there are errors/warnings.
bool
SYSLOG::Close( bool inForce )
{
  if( !inForce && mDontClose && mpForm->Visible )
    return false;

  mpForm->Close();
  return true;
}

// **************************************************************************
// Function:   ShowSysLog
// Purpose:    Shows the system log in a window
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
SYSLOG::ShowSysLog()
{
  mpForm->Show();
}

// **************************************************************************
// Function:   HideSysLog
// Purpose:    Hides the system log window
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
SYSLOG::HideSysLog()
{
  mpForm->Hide();
}

// **************************************************************************
// Function:   Visible
// Purpose:    Returns whether the sys log window is visible
// Parameters: N/A
// Returns:    true if the window is visible
// **************************************************************************
bool
SYSLOG::Visible() const
{
  return mpForm->Visible;
}

// **************************************************************************
// Function:   AddSysLogEntry
// Purpose:    Adds an entry to the system log with a special attribute
// Parameters: text - pointer to the text to add to the log
//             mode - either logEntryNormal  (normal attributes)
//                           logEntryWarning (attributes for a warning message)
//                           logEntryError   (attributes for an error message)
// Returns:    the line entered into the system log
// **************************************************************************
const char*
SYSLOG::AddSysLogEntry( const char* inText, LogEntryMode inMode )
{
  mpCritsec->Acquire();
  switch( inMode )
  {
    case logEntryWarning:
      mpLog->SelAttributes->Color = clGreen;
      mpLog->SelAttributes->Height += 2;
      mpLog->SelAttributes->Style = mpLog->SelAttributes->Style << fsBold;
      mDontClose = true;
      break;

    case logEntryError:
      mpLog->SelAttributes->Color = clRed;
      mpLog->SelAttributes->Height += 2;
      mpLog->SelAttributes->Style = mpLog->SelAttributes->Style << fsBold;
      mDontClose = true;
      break;

    case logEntryNormal:
    default:
      break;
  }
  mLastLine = TDateTime::CurrentDateTime().FormatString( "ddddd tt - " ) + AnsiString( inText );
  mpLog->Lines->Add( mLastLine );
  mpLog->SelAttributes->Assign( mpLog->DefAttributes );
  mpCritsec->Release();
  return mLastLine.c_str();
}

