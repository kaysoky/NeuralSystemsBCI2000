/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
// MFCdemoDlg.h : Header-Datei
//

#if !defined(AFX_MFCDEMODLG_H__F7408965_B810_4100_A7B8_450D96E11AE9__INCLUDED_)
#define AFX_MFCDEMODLG_H__F7408965_B810_4100_A7B8_450D96E11AE9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMFCdemoDlg Dialogfeld

class CMFCdemoDlg : public CDialog
{
// Konstruktion
public:
	CMFCdemoDlg(CWnd* pParent = NULL);	// Standard-Konstruktor

// Dialogfelddaten
	//{{AFX_DATA(CMFCdemoDlg)
	enum { IDD = IDD_MFCDEMO_DIALOG };
		// HINWEIS: der Klassenassistent fügt an dieser Stelle Datenelemente (Members) ein
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CMFCdemoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
public:
  void  SetCursorX( float x ) { mCursorX = x; }
  float CursorX() const   { return mCursorX; }
  void  SetCursorY( float y ) { mCursorY = y; }
  float CursorY() const   { return mCursorY; }
private:
  float mCursorX,
        mCursorY;
  HICON mCursorIcon;

protected:
	HICON m_hIcon;

	// Generierte Message-Map-Funktionen
	//{{AFX_MSG(CMFCdemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_MFCDEMODLG_H__F7408965_B810_4100_A7B8_450D96E11AE9__INCLUDED_)
