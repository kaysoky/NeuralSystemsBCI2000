////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A window that contains formatted text.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "TextWindow.h"


TextWindow::TextWindow()
: mpForm( new TForm( static_cast<TComponent*>( NULL ) ) ),
  mpEditField( new TEdit( mpForm ) )
{
  mpForm->Visible = false;
  mpEditField->Parent = mpForm;
  mpEditField->Align = alClient;
}

TextWindow::~TextWindow()
{
  delete mpForm;
}

TextWindow&
TextWindow::Show()
{
  mpForm->Show();
  return *this;
}

TextWindow&
TextWindow::Hide()
{
  mpForm->Show();
  return *this;
}


TextWindow&
TextWindow::SetLeft( int inLeft )
{
  mpForm->Left = inLeft;
  return *this;
}

int
TextWindow::Left() const
{
  return mpForm->Left;
}

TextWindow&
TextWindow::SetTop( int inTop )
{
  mpForm->Top = inTop;
  return *this;
}

int
TextWindow::Top() const
{
  return mpForm->Top;
}

TextWindow&
TextWindow::SetWidth( int inWidth )
{
  mpForm->Width = inWidth;
  return *this;
}

int
TextWindow::Width() const
{
  return mpForm->Width;
}

TextWindow&
TextWindow::SetHeight( int inHeight )
{
  mpForm->Height = inHeight;
  return *this;
}

int
TextWindow::Height() const
{
  return mpForm->Height;
}


TextWindow&
TextWindow::SetText( const std::string& inText )
{
  mpEditField->Text = inText.c_str();
  return *this;
}

const std::string&
TextWindow::Text() const
{
  mTextBuf = mpEditField->Text.c_str();
  return mTextBuf;
}


TextWindow&
TextWindow::SetFontName( const std::string& inFontName )
{
  mpEditField->Font->Name = inFontName.c_str();
  return *this;
}

const std::string&
TextWindow::FontName() const
{
  mTextBuf = mpEditField->Font->Name.c_str();
  return mTextBuf;
}

TextWindow&
TextWindow::SetFontSize( int inSize )
{
  mpEditField->Font->Size = inSize;
  return *this;
}

int
TextWindow::FontSize() const
{
  return mpEditField->Font->Size;
}


