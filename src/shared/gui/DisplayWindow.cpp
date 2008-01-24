////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A GraphDisplay descendant which is a frameless GUI window for
//   an application's user display.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "DisplayWindow.h"

using namespace GUI;

#ifdef __BORLANDC__
DisplayWindow::DisplayWindow()
: mpForm( new TDisplayForm( *this ) ),
  mWinDC( NULL )
{
  UpdateContext();
  mpForm->BorderStyle = bsNone;
}

DisplayWindow::~DisplayWindow()
{
  delete mpForm;
}

DisplayWindow&
DisplayWindow::SetLeft( int inLeft )
{
  mpForm->Left = inLeft;
  return UpdateContext();
}

int
DisplayWindow::Left() const
{
  return mpForm->Left;
}

DisplayWindow&
DisplayWindow::SetTop( int inTop )
{
  mpForm->Top = inTop;
  return UpdateContext();
}

int
DisplayWindow::Top() const
{
  return mpForm->Top;
}

DisplayWindow&
DisplayWindow::SetWidth( int inWidth )
{
  mpForm->Width = inWidth;
  return UpdateContext();
}

int
DisplayWindow::Width() const
{
  return mpForm->Width;
}

DisplayWindow&
DisplayWindow::SetHeight( int inHeight )
{
  mpForm->Height = inHeight;
  return UpdateContext();
}

int
DisplayWindow::Height() const
{
  return mpForm->Height;
}

DisplayWindow&
DisplayWindow::Show()
{
  mpForm->Show();
  ::ReleaseDC( mpForm->Handle, mWinDC );
  mWinDC = ::GetDC( mpForm->Handle );
  UpdateContext();
  return *this;
}

DisplayWindow&
DisplayWindow::Hide()
{
  mpForm->Hide();
  return *this;
}

bool
DisplayWindow::Visible() const
{
  return mpForm->Visible;
}

DisplayWindow&
DisplayWindow::UpdateContext()
{
  DrawContext dc =
  {
    mWinDC,
    { 0, 0, mpForm->ClientWidth, mpForm->ClientHeight }
  };
  GraphDisplay::SetContext( dc );
  return *this;
}
#endif // __BORLANDC__

