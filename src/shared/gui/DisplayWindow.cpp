////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de, griffin.milsap@gmail.com
// Description: A GraphDisplay descendant which is a frameless GUI window for
//   an application's user display.
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

#ifdef __BORLANDC__
# include "VCLdefines.h"
#else // __BORLANDC__
# include <QApplication>
#endif // __BORLANDC__

#include "DisplayWindow.h"

using namespace GUI;
using namespace std;

DisplayWindow::DisplayWindow()
: mTop( 0 ),
  mLeft( 0 ),
  mWidth( 0 ),
  mHeight( 0 ),
  mpForm( NULL )
{
#ifdef __BORLANDC__
  mWinDC = NULL;
  mTitle = AnsiString( Application->Title ).c_str();
#else // __BORLANDC__
  mTitle = qApp->applicationName().toLocal8Bit().constData();
#endif // __BORLANDC__
  Restore();
  UpdateContext();
}

DisplayWindow::~DisplayWindow()
{
  Clear();
}

DisplayWindow&
DisplayWindow::SetTitle( const string& inTitle )
{
  mTitle = inTitle;
  if( mpForm != NULL )
  {
#ifdef __BORLANDC__
    mpForm->Caption = VCLSTR( mTitle.c_str() );
#else // __BORLANDC__
    mpForm->setWindowTitle( QString::fromLocal8Bit( mTitle.c_str() ) );
#endif // __BORLANDC__
  }
  return *this;
}

const string&
DisplayWindow::Title() const
{
  return mTitle;
}

DisplayWindow&
DisplayWindow::SetLeft( int inLeft )
{
  mLeft = inLeft;
  if( mpForm != NULL )
  {
#ifdef __BORLANDC__
    mpForm->Left = mLeft;
#else // __BORLANDC__
    mpForm->move( mLeft, mTop );
#endif
    UpdateContext();
  }

  return *this;
}

int
DisplayWindow::Left() const
{
  return mLeft;
}

DisplayWindow&
DisplayWindow::SetTop( int inTop )
{
  mTop = inTop;
  if( mpForm != NULL )
  {
#ifdef __BORLANDC__
    mpForm->Top = inTop;
#else // __BORLANDC__
    mpForm->move( mLeft, mTop );
#endif //__BORLANDC__
    UpdateContext();
  }
  return *this;
}

int
DisplayWindow::Top() const
{
  return mTop;
}

DisplayWindow&
DisplayWindow::SetWidth( int inWidth )
{
  mWidth = inWidth;
  if( mpForm != NULL )
  {
#ifdef __BORLANDC__
    mpForm->Width = inWidth;
#else // __BORLANDC__
    mpForm->resize( mWidth, mHeight );
#endif // __BORLANDC__
    UpdateContext();
  }
  return *this;
}

int
DisplayWindow::Width() const
{
  return mWidth;
}

DisplayWindow&
DisplayWindow::SetHeight( int inHeight )
{
  mHeight = inHeight;
  if( mpForm != NULL )
  {
#ifdef __BORLANDC__
    mpForm->Height = inHeight;
#else //__BORLANDC__
    mpForm->resize( mWidth, mHeight );
#endif // __BORLANDC__
    UpdateContext();
  }
  return *this;
}

int
DisplayWindow::Height() const
{
  return mHeight;
}

DisplayWindow&
DisplayWindow::Show()
{
#ifdef __BORLANDC__
  if( mpForm && mpForm->Visible )
    return *this;
#endif // __BORLANDC__
  if( mpForm == NULL )
    Restore();
#ifdef __BORLANDC__
  mpForm->Show();
#else // __BORLANDC__
  mpForm->show();
#endif // __BORLANDC__
#ifdef __BORLANDC__
  HDC oldDC = mWinDC;
  mWinDC = ::GetDC( mpForm->Handle );
#endif // __BORLANDC__
  UpdateContext();
#ifdef __BORLANDC__
  ::ReleaseDC( mpForm->Handle, oldDC );
#endif // __BORLANDC__
  return *this;
}

DisplayWindow&
DisplayWindow::Hide()
{
#ifdef __BORLANDC__
  return Clear();
#else // __BORLANDC__
  mpForm->hide();
  return *this;
#endif // __BORLANDC__
}

bool
DisplayWindow::Visible() const
{
#ifdef __BORLANDC__
  return mpForm ? mpForm->Visible : false;
#else // __BORLANDC__
  return mpForm ? mpForm->isVisible() : false;
#endif // __BORLANDC__
}

DisplayWindow&
DisplayWindow::Restore()
{
#ifdef __BORLANDC__
  mpForm = new TDisplayForm( *this );
  mpForm->Left = mLeft;
  mpForm->Top = mTop;
  mpForm->Height = mHeight;
  mpForm->Width = mWidth;
  mpForm->Caption = VCLSTR( mTitle.c_str() );
#else // __BORLANDC__
  mpForm = new QWidget;
  mpForm->setWindowFlags( Qt::FramelessWindowHint );
  mpForm->setAttribute( Qt::WA_NoSystemBackground );
  mpForm->move( mLeft, mTop );
  mpForm->resize( mWidth, mHeight );
  mpForm->setWindowTitle( QString::fromLocal8Bit( mTitle.c_str() ) );
#endif // __BORLANDC__
  return *this;
}

DisplayWindow&
DisplayWindow::Clear()
{
#ifdef __BORLANDC__
  HDC oldDC = mWinDC;
  mWinDC = NULL;
  UpdateContext();
  ::ReleaseDC( mpForm->Handle, oldDC );
#endif // __BORLANDC__
  delete mpForm;
  mpForm = NULL;
#ifndef __BORLANDC__
  UpdateContext();
#endif // __BORLANDC__
  return *this;
}

DisplayWindow&
DisplayWindow::UpdateContext()
{
#ifdef __BORLANDC__
  DrawContext dc =
  {
    mWinDC,
    { 0, 0, mWidth, mHeight }
  };
#else // __BORLANDC__
  DrawContext dc =
  {
    { mpForm, NULL, NULL },
    { 0, 0, mWidth, mHeight }
  };
#endif // __BORLANDC__
  GraphDisplay::SetContext( dc );
  return *this;
}

