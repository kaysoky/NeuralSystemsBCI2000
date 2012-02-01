////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A window that contains formatted text.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "TextWindow.h"

#if __BORLANDC__
TextWindow::TextWindow()
: mpForm( new TForm( static_cast<TComponent*>( NULL ) ) ),
  mpEditField( new TEdit( mpForm ) )
{
  mpForm->Visible = false;
  mpEditField->Parent = mpForm;
  mpEditField->Align = alClient;
}
#else // __BORLANDC__
TextWindow::TextWindow()
: mpForm( new QWidget() ),
  mpEditField( new QTextEdit( mpForm ) )
{
  QHBoxLayout* pLayout = new QHBoxLayout( mpForm );
  pLayout->addWidget( mpEditField );
  pLayout->setContentsMargins( 0, 0, 0, 0 );
  mpForm->setVisible( false );
}
#endif // __BORLANDC__

TextWindow::~TextWindow()
{
  delete mpForm;
}

TextWindow&
TextWindow::Show()
{
#if __BORLANDC__
  mpForm->Show();
#else // __BORLANDC__
  mpForm->show();
#endif // __BORLANDC__
  return *this;
}

TextWindow&
TextWindow::Hide()
{
#if __BORLANDC__
  mpForm->Hide();
#else // __BORLANDC__
  mpForm->hide();
#endif // __BORLANDC__
  return *this;
}


TextWindow&
TextWindow::SetLeft( int inLeft )
{
#if __BORLANDC__
  mpForm->Left = inLeft;
#else // __BORLANDC__
  mpForm->move( inLeft, mpForm->y() );
#endif // __BORLANDC__
  return *this;
}

int
TextWindow::Left() const
{
#if __BORLANDC__
  return mpForm->Left;
#else // __BORLANDC__
  return mpForm->x();
#endif // __BORLANDC__
}

TextWindow&
TextWindow::SetTop( int inTop )
{
#if __BORLANDC__
  mpForm->Top = inTop;
#else // __BORLANDC__
  mpForm->move( mpForm->x(), inTop );
#endif // __BORLANDC__
  return *this;
}

int
TextWindow::Top() const
{
#if __BORLANDC__
  return mpForm->Top;
#else // __BORLANDC__
  return mpForm->y();
#endif // __BORLANDC__
}

TextWindow&
TextWindow::SetWidth( int inWidth )
{
#if __BORLANDC__
  mpForm->Width = inWidth;
#else // __BORLANDC__
  mpForm->resize( inWidth, mpForm->height() );
#endif // __BORLANDC__
  return *this;
}

int
TextWindow::Width() const
{
#if __BORLANDC__
  return mpForm->Width;
#else // __BORLANDC__
  return mpForm->width();
#endif // __BORLANDC__
}

TextWindow&
TextWindow::SetHeight( int inHeight )
{
#if __BORLANDC__
  mpForm->Height = inHeight;
#else // __BORLANDC__
  mpForm->resize( mpForm->width(), inHeight );
#endif // __BORLANDC__
  return *this;
}

int
TextWindow::Height() const
{
#if __BORLANDC__
  return mpForm->Height;
#else // __BORLANDC__
  return mpForm->height();
#endif // __BORLANDC__
}


TextWindow&
TextWindow::SetText( const std::string& inText )
{
#if __BORLANDC__
  mpEditField->Text = inText.c_str();
#else // __BORLANDC__
  mpEditField->setText( QString::fromLocal8Bit( inText.c_str() ) );
  mpEditField->moveCursor( QTextCursor::End );
#endif // __BORLANDC__
  return *this;
}

const std::string&
TextWindow::Text() const
{
#if __BORLANDC__
  mTextBuf = AnsiString( mpEditField->Text ).c_str();
#else // __BORLANDC__
  mTextBuf = mpEditField->toPlainText().toLocal8Bit().constData();
#endif // __BORLANDC__
  return mTextBuf;
}


TextWindow&
TextWindow::SetFontName( const std::string& inFontName )
{
#if __BORLANDC__
  mpEditField->Font->Name = inFontName.c_str();
#else // __BORLANDC__
  mpEditField->setFontFamily( QString::fromLocal8Bit( inFontName.c_str() ) );
#endif // __BORLANDC__
  return *this;
}

const std::string&
TextWindow::FontName() const
{
#if __BORLANDC__
  mTextBuf = AnsiString( mpEditField->Font->Name ).c_str();
#else // __BORLANDC__
  mTextBuf = mpEditField->font().family().toLocal8Bit().constData();
#endif // __BORLANDC__
  return mTextBuf;
}

TextWindow&
TextWindow::SetFontSize( int inSize )
{
#if __BORLANDC__
  mpEditField->Font->Size = inSize;
#else // __BORLANDC__
  mpEditField->setFontPointSize( inSize );
#endif // __BORLANDC__
  return *this;
}

float
TextWindow::FontSize() const
{
#if __BORLANDC__
  return mpEditField->Font->Size;
#else // __BORLANDC__
  return mpEditField->fontPointSize();
#endif // __BORLANDC__
}


