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
  mpLog( new TRichEdit( ( TComponent* )NULL ) ),
  mpCritsec( new TCriticalSection ),
  mTextHeight( 0 ),
  mDontClose( false )
{
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
  mTextHeight = mpLog->SelAttributes->Height;
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
  delete mpLog;
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
// Returns:    N/A
// **************************************************************************
void
SYSLOG::AddSysLogEntry( const char* inText, LogEntryMode inMode )
{
  mpCritsec->Acquire();
  switch( inMode )
  {
    case logEntryWarning:
      mpLog->SelAttributes->Color = clGreen;
      mpLog->SelAttributes->Height = mTextHeight + 2;
      mpLog->SelAttributes->Style << fsBold;
      mDontClose = true;
      break;

    case logEntryError:
      mpLog->SelAttributes->Color = clRed;
      mpLog->SelAttributes->Height = mTextHeight + 2;
      mpLog->SelAttributes->Style << fsBold;
      mDontClose = true;
      break;
      
    case logEntryNormal:
    default:
      mpLog->SelAttributes->Color = clBlack;
      mpLog->SelAttributes->Height = mTextHeight;
      mpLog->SelAttributes->Style >> fsBold;
      break;
  }
  mpLog->Lines->Add(
    TDateTime::CurrentDateTime().FormatString( "ddddd tt - " ) + AnsiString( inText ) );
  mpCritsec->Release();
}

