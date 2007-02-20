// MFCdemo.cpp : Legt das Klassenverhalten für die Anwendung fest.
//

#include "stdafx.h"
#include "MFCdemo.h"
#include "MFCdemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCdemoApp

BEGIN_MESSAGE_MAP(CMFCdemoApp, CWinApp)
	//{{AFX_MSG_MAP(CMFCdemoApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////
// CMFCdemoApp Konstruktion

CMFCdemoApp::CMFCdemoApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// Das einzige CMFCdemoApp-Objekt

CMFCdemoApp theApp;
