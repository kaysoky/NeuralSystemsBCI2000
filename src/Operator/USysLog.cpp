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
 // form->BorderIcons >> biSystemMenu;
 // form->BorderStyle=bsSizeable;

 log=new TMemo(form);
 log->Visible=false;
 log->Parent=form;
 log->Left=0;
 log->Top=0;
 log->Width=form->ClientWidth;
 log->Height=form->ClientHeight;
 log->Anchors << akLeft << akTop << akRight << akBottom;
 log->ScrollBars=ssVertical;
 log->Visible=true;
}


// **************************************************************************
// Function:   SYSLOG
// Purpose:    The destructor for the SYSLOG class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SYSLOG::~SYSLOG()
{
 if (log)  delete log;
 if (form) delete form;

 form=NULL;
 log=NULL;
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

 cur_time=cur_time.CurrentDateTime();
 log->Lines->Add(cur_time.FormatString("ddddd tt - ")+AnsiString(text));
}

