//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

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
 if (log)     delete log;
 if (form)    delete form;
 if (critsec) delete critsec;

 form=NULL;
 log=NULL;
 critsec=NULL;
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
// Function:   AddSysLogEntry
// Purpose:    Adds an entry to the system log
// Parameters: text - pointer to the text to add to the log
// Returns:    N/A
// **************************************************************************
void SYSLOG::AddSysLogEntry(char *text)
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
void SYSLOG::AddSysLogEntry(char *text, int mode)
{
TDateTime       cur_time;

 critsec->Acquire();

 // is this a warning message ?
 if (mode == SYSLOGENTRYMODE_WARNING)
    {
    log->SelAttributes->Color = clGreen;
    log->SelAttributes->Height += 2;
    log->SelAttributes->Style = log->SelAttributes->Style << fsBold;
    }
 // is this an error message ?
 if (mode == SYSLOGENTRYMODE_ERROR)
    {
    log->SelAttributes->Color = clRed;
    log->SelAttributes->Height += 2;
    log->SelAttributes->Style = log->SelAttributes->Style << fsBold;
    }

 cur_time=cur_time.CurrentDateTime();
 log->Lines->Add(cur_time.FormatString("ddddd tt - ")+AnsiString(text));

 critsec->Release();
}

