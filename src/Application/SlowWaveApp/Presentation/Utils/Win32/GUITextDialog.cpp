/////////////////////////////////////////////////////////////////////////////
//
// File: GUITextDialog.cpp
//
// Date: Nov 22, 2001
//
// Author: Juergen Mellinger
//
// Description: A class that opens a GUI window and displays text inside it.
//
// Changes:
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "OSIncludes.h"

#ifndef VCL
# error This is the VCL implementation of TGUITextDialog.
#endif

#include "../GUITextDialog.h"

TGUITextDialog::TGUITextDialog()
: form( NULL )
{
}

TGUITextDialog::~TGUITextDialog()
{
  try
  {
    // Sometimes the window handle is gone before this is called ...
    delete form;
  }
  catch( ... )
  {
  }
}

void
TGUITextDialog::ShowTextNonmodal(   const char  *inTitle,
                                    const char  *inText )
{
    const char  *p = inText;
    AnsiString  text;

    while( *p != '\0' )
    {
        if( *p == '\n' )
            text += '\r';
        text += *p;
        ++p;
    }

    if( form == NULL )
        form = new TTextForm;
    form->FormStyle = fsStayOnTop;
    form->Caption = inTitle;
    form->memo->Text = form->memo->Text + text;
    form->memo->SelStart = form->memo->Text.Length();
    SendMessage( form->memo->Handle, WM_VSCROLL, SB_BOTTOM, 0 );
    form->Refresh();
    form->Show();
    // This avoids unwanted interaction with message boxes
    // while working around the BringToFront mess.
    form->FormStyle = fsNormal;
}

void
TGUITextDialog::ShowTextModal(  const char  *inTitle,
                                const char  *inText )
{
    const char  *p = inText;
    AnsiString  text;

    while( *p != '\0' )
    {
        if( *p == '\n' )
            text += '\r';
        text += *p;
        ++p;
    }

    TTextForm   *modalForm = new TTextForm;
    modalForm->Caption = inTitle;
    modalForm->ShowModal();
    delete modalForm;
}

__fastcall
TGUITextDialog::TTextForm::TTextForm()
: TForm( ( TComponent* )NULL, 1 )
{
    if( HICON iconHandle = ::LoadIcon( NULL, IDI_EXCLAMATION ) )
    {
        Icon->ReleaseHandle();
        Icon->Handle = iconHandle;
    }

    const int frameWidth = 4;

    Left = ( Screen->Width - Width ) / 2;
    Top = ( Screen->Height - Height ) / 2;
    OnClose = FormCloseHandler;

    TButton *button = new TButton( this );
    button->OnClick = OKButtonClickHandler;
    button->Default = true;
    button->Cancel = true;
    button->Caption = "OK";
    button->Left = ClientWidth - button->Width - frameWidth;
    button->Top = ClientHeight - button->Height - frameWidth;
    button->Anchors.Clear();
    button->Anchors << akRight << akBottom;
    button->Parent = this;

    memo = new TMemo( this );
    memo->Left = frameWidth;
    memo->Top = frameWidth;
    memo->Height = ClientHeight - button->Height - 3 * frameWidth;
    memo->Width = ClientWidth - 2 * frameWidth;
    memo->Anchors << akLeft << akRight << akTop << akBottom;
    memo->ReadOnly = true;
    memo->ScrollBars = ssVertical;
    memo->Parent = this;
}

void
__fastcall
TGUITextDialog::TTextForm::OKButtonClickHandler( TObject* )
{
    Close();
}

void
__fastcall
TGUITextDialog::TTextForm::FormCloseHandler( TObject*, TCloseAction &outAction )
{
    memo->Text = "";
    outAction = caHide;
}

