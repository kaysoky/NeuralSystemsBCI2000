// MFCdemo.h : Haupt-Header-Datei für die Anwendung MFCDEMO
//

#if !defined(AFX_MFCDEMO_H__84343563_3F97_4804_B2CD_924D5BDF6726__INCLUDED_)
#define AFX_MFCDEMO_H__84343563_3F97_4804_B2CD_924D5BDF6726__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// Hauptsymbole

/////////////////////////////////////////////////////////////////////////////
// CMFCdemoApp:
// Siehe MFCdemo.cpp für die Implementierung dieser Klasse
//

class CMFCdemoApp : public CWinApp
{
public:
	CMFCdemoApp();

// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CMFCdemoApp)
	public:
	//}}AFX_VIRTUAL

// Implementierung

	//{{AFX_MSG(CMFCdemoApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_MFCDEMO_H__84343563_3F97_4804_B2CD_924D5BDF6726__INCLUDED_)
