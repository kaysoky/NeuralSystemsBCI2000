/////////////////////////////////////////////////////////////////////////////
//
// File: GUITextDialog.cpp
//
// Date: Jan 31, 2004
//
// Author: Juergen Mellinger
//
// Description: A class that opens a GUI window and displays text inside it.
//              Implementation for the BCI2000 bcierr framework class.
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "OSIncludes.h"

#ifndef BCI2000
# error This is the BCI2000 implementation of TGUITextDialog.
#endif

#include "../GUITextDialog.h"
#include "BCIError.h"

using namespace std;

TGUITextDialog::TGUITextDialog()
{
}

TGUITextDialog::~TGUITextDialog()
{
}

void
TGUITextDialog::ShowTextNonmodal( const char  *inTitle,
                                  const char  *inText )
{
  bcierr__ << inTitle
           << ": "
           << inText
           << endl;
}

void
TGUITextDialog::ShowTextModal( const char  *inTitle,
                               const char  *inText )
{
  bcierr__ << "[Should be a modal dialog window] "
           << inTitle
           << ": "
           << inText
           << endl;
}

