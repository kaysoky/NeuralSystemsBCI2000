#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "USysLog.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

// **************************************************************************
// Function:   SYSLOG
// Purpose:    The constructor for the SYSLOG class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SYSLOG::SYSLOG()
: dontClose( false )
{
 form=new TForm(Application);
 form->Caption="System Log";
 form->Width=600;
 form->Height=250;
 // form->BorderIcons >> biSystemMenu;
 // form->BorderStyle=bsSizeable;

 log=new TRichEdit(form);
 log->Visible=false;
 log->Parent=form;
 log->Left=0;
 log->Top=0;
 log->Width=form->ClientWidth;
 log->Height=form->ClientHeight;
 log->Anchors << akLeft << akTop << akRight << akBottom;
 log->ScrollBars=ssVertical;
 log->Visible=true;

 critsec=new TCriticalSection();
}


// **************************************************************************
// Function:   SYSLOG
// Purpose:    The destructor for the SYSLOG class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SYSLOG::~SYSLOG()
{
 delete critsec;
}

// The user must close the syslog manually if there are errors/warnings.
bool SYSLOG::Close( bool force )
{
 if( !force && dontClose && form->Visible )
   return false;

 form->Close();
 return true;
}

// **************************************************************************
// Function:   ShowSysLog
// Purpose:    Shows the system log in a window
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void SYSLOG::ShowSysLog()
{
 form->Show();
}

// **************************************************************************
// Function:   HideSysLog
// Purpose:    Hides the system log window
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void SYSLOG::HideSysLog()
{
 form->Hide();
}

// **************************************************************************
// Function:   Visible
// Purpose:    Returns whether the sys log window is visible
// Parameters: N/A
// Returns:    true if the window is visible
// **************************************************************************
bool SYSLOG::Visible() const
{
 return form->Visible;
}

// **************************************************************************
// Function:   AddSysLogEntry
// Purpose:    Adds an entry to the system log
// Parameters: text - pointer to the text to add to the log
// Returns:    N/A
// **************************************************************************
void SYSLOG::AddSysLogEntry(const char *text)
{
TDateTime       cur_time;

 critsec->Acquire();
 cur_time=cur_time.CurrentDateTime();
 log->Lines->Add(cur_time.FormatString("ddddd tt - ")+AnsiString(text));
 critsec->Release();
}

// **************************************************************************
// Function:   AddSysLogEntry
// Purpose:    Adds an entry to the system log with a special attribute
// Parameters: text - pointer to the text to add to the log
//             mode - either SYSLOGENTRYMODE_NORMAL  (normal attributes)
//                           SYSLOGENTRYMODE_WARNING (attributes for a warning message)
//                           SYSLOGENTRYMODE_ERROR   (attributes for an error message)
// Returns:    N/A
// **************************************************************************
void SYSLOG::AddSysLogEntry(const char *text, int mode)
{
TDateTime       cur_time;

 critsec->Acquire();

 // is this a warning message ?
 if (mode == SYSLOGENTRYMODE_WARNING)
    {
    log->SelAttributes->Color = clGreen;
    log->SelAttributes->Height += 2;
    log->SelAttributes->Style = log->SelAttributes->Style << fsBold;
    dontClose = true;
    }
 // is this an error message ?
 if (mode == SYSLOGENTRYMODE_ERROR)
    {
    log->SelAttributes->Color = clRed;
    log->SelAttributes->Height += 2;
    log->SelAttributes->Style = log->SelAttributes->Style << fsBold;
    dontClose = true;
    }

 cur_time=cur_time.CurrentDateTime();
 log->Lines->Add(cur_time.FormatString("ddddd tt - ")+AnsiString(text));

 critsec->Release();
}

