////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A GraphDisplay descendant which is a frameless GUI window for
//   an application's user display.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "DisplayWindow.h"

using namespace GUI;

#ifdef __BORLANDC__
DisplayWindow::DisplayWindow()
: mpForm( NULL ),
  mWinDC( NULL )
{
  Restore();
  UpdateContext();
}

DisplayWindow::~DisplayWindow()
{
  Clear();
}

DisplayWindow&
DisplayWindow::SetLeft( int inLeft )
{
  mLeft = inLeft;
  if( mpForm != NULL )
  {
    mpForm->Left = mLeft;
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
    mpForm->Top = inTop;
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
    mpForm->Width = inWidth;
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
    mpForm->Height = inHeight;
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
  if( mpForm == NULL )
    Restore();
  mpForm->Show();
  HDC oldDC = mWinDC;
  mWinDC = ::GetDC( mpForm->Handle );
  UpdateContext();
  ::ReleaseDC( mpForm->Handle, oldDC );
  return *this;
}

DisplayWindow&
DisplayWindow::Hide()
{
  return Clear();
}

bool
DisplayWindow::Visible() const
{
  return mpForm ? mpForm->Visible : false;
}

DisplayWindow&
DisplayWindow::Restore()
{
  mpForm = new TDisplayForm( *this );
  mpForm->BorderStyle = bsNone;
  mpForm->Left = mLeft;
  mpForm->Top = mTop;
  mpForm->Height = mHeight;
  mpForm->Width = mWidth;
  return *this;
}

DisplayWindow&
DisplayWindow::Clear()
{
  HDC oldDC = mWinDC;
  mWinDC = NULL;
  UpdateContext();
  ::ReleaseDC( mpForm->Handle, oldDC );
  delete mpForm;
  mpForm = NULL;
  return *this;
}


DisplayWindow&
DisplayWindow::UpdateContext()
{
  DrawContext dc =
  {
    mWinDC,
    { 0, 0, mWidth, mHeight }
  };
  GraphDisplay::SetContext( dc );
  return *this;
}
#endif // __BORLANDC__

