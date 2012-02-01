////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A GraphObject consisting of two stacked text fields
//   separated with a separator bar.
//   GoalText is the top field, ResultText is the bottom field.
//   For an empty GoalText, ResultText occupies the top as well, and the
//   separator bar is not displayed.
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

#include "StatusBar.h"

#ifdef __BORLANDC__
#include "VCL.h"
#else // __BORLANDC__
#include <QPainter>
#include <QFontMetrics>
#endif // __BORLANDC__

using namespace std;
using namespace GUI;

StatusBar::StatusBar( GraphDisplay& display )
: GraphObject( display, StatusBarZOrder ),
  mTextHeight( 1.0 ),
  mColor( RGBColor::NullColor ),
  mTextColor( RGBColor::Gray )
{
}

StatusBar::~StatusBar()
{
}

StatusBar&
StatusBar::SetGoalText( const string& s )
{
  mGoalText = s;
  Change();
  return *this;
}

const string& StatusBar::GoalText() const
{
  return mGoalText;
}

StatusBar&
StatusBar::SetResultText( const string& s )
{
  mResultText = s;
  Change();
  return *this;
}

const string& StatusBar::ResultText() const
{
  return mResultText;
}

StatusBar&
StatusBar::SetTextHeight( float f )
{
  mTextHeight = f;
  Change();
  return *this;
}

float
StatusBar::TextHeight() const
{
  return mTextHeight;
}

StatusBar&
StatusBar::SetTextColor( RGBColor c )
{
  mTextColor = c;
  Change();
  return *this;
}

RGBColor
StatusBar::TextColor() const
{
  return mTextColor;
}

StatusBar&
StatusBar::SetColor( RGBColor c )
{
  mColor = c;
  Change();
  return *this;
}

RGBColor
StatusBar::Color() const
{
  return mColor;
}

void
StatusBar::OnPaint( const DrawContext& dc )
{
  const int heightRatioGoal = 1,
            heightRatioResult = 1;
  RGBColor  dividerColor = RGBColor::Black,
            backgroundColor = mColor,
            textColor = mTextColor;
  string    line1, line2;

  if( mGoalText.empty() )
  {
    size_t spacePos = mResultText.rfind( ' ' );
    if( spacePos != string::npos )
    {
      line1 = mResultText.substr( 0, spacePos );
      line2 = mResultText.substr( spacePos + 1, string::npos );
    }
    else
    {
      line1 = "";
      line2 = mResultText;
    }
  }
  else
  {
    line1 = mGoalText;
    line2 = mResultText;
  }

#ifdef __BORLANDC__
  TCanvas* pCanvas = new TCanvas;
  try
  {
    pCanvas->Handle = ( HDC )dc.handle;
    TRect winRect( dc.rect.left, dc.rect.top, dc.rect.right, dc.rect.bottom );
    if( backgroundColor != RGBColor::NullColor )
    {
      pCanvas->Brush->Style = bsSolid;
      pCanvas->Brush->Color = TColor( backgroundColor.ToWinColor() );
      pCanvas->FillRect( winRect );
    }
    pCanvas->Font->Name = "Arial";
    pCanvas->Font->Height = -mTextHeight * ( dc.rect.bottom - dc.rect.top );
    pCanvas->Font->Style = TFontStyles() << fsBold;
    pCanvas->Font->Color = TColor( textColor.ToWinColor() );
    int charWidth = pCanvas->TextWidth( "x" );

    int dividerPos = winRect.top + ( winRect.Height() * heightRatioGoal )
                                  / ( heightRatioGoal + heightRatioResult );
    if( !mGoalText.empty() )
    {
      TRect dividerRect = winRect;
      dividerRect.top = dividerPos;
      dividerRect.bottom = dividerPos + 1;
      pCanvas->Brush->Color = TColor( dividerColor.ToWinColor() );
      pCanvas->FillRect( dividerRect );
    }

    AnsiString text = line1.c_str();
    TRect line1Rect = winRect;
    line1Rect.bottom = dividerPos;
    TSize size = pCanvas->TextExtent( text );
    while( size.cx > line1Rect.Width() - 2 * charWidth )
    {
      text = text.SubString( 2, text.Length() );
      size = pCanvas->TextExtent( text );
    }
    int xPos = line1Rect.left + charWidth,
        yPos = line1Rect.top + ( line1Rect.Height() - size.cy ) / 2;
    pCanvas->Brush->Style = bsClear;
    pCanvas->TextRect( line1Rect, xPos, yPos, text );

    text = line2.c_str();
    TRect line2Rect = winRect;
    line2Rect.top = dividerPos;
    size = pCanvas->TextExtent( text );
    while( size.cx > line2Rect.Width() - 2 * charWidth )
    {
      text = text.SubString( 2, text.Length() );
      size = pCanvas->TextExtent( text );
    }
    xPos = line2Rect.left + charWidth;
    yPos = line2Rect.top + ( line2Rect.Height() - size.cy ) / 2;
    pCanvas->Brush->Style = bsClear;
    pCanvas->TextRect( line2Rect, xPos, yPos, text );
  }
  __finally
  {
    delete pCanvas;
  }
#else // __BORLANDC__
  // Create the painter
  QPainter* p = dc.handle.painter;

  // Paint the background if it exists
  if( backgroundColor != RGBColor( RGBColor::NullColor ) )
  {
    QColor colorBackground( backgroundColor.R(), backgroundColor.G(), backgroundColor.B() );
    QBrush brushBackground( colorBackground );
    p->fillRect(
      static_cast<int>( dc.rect.left ),
      static_cast<int>( dc.rect.top ),
      static_cast<int>( dc.rect.right - dc.rect.left ),
      static_cast<int>( dc.rect.bottom - dc.rect.top ),
      brushBackground
    );
  }

  // Set up font
  QFont font;
  font.fromString( QString( "Arial" ) );
  font.setPixelSize( static_cast<int>( mTextHeight * ( dc.rect.bottom - dc.rect.top ) ) );
  font.setBold( true );
  p->setFont( font );

  // Color the font
  QPen fontPen;
  fontPen.setColor( QColor( textColor.R(), textColor.G(), textColor.B() ) );
  p->setPen( fontPen );

  QFontMetrics fm( font );
  int charWidth = fm.width( QString( "x" ) );
  int dividerPos = static_cast<int>( dc.rect.top + ( ( dc.rect.bottom - dc.rect.top ) * heightRatioGoal )
                                     / ( heightRatioGoal + heightRatioResult ) );

  if( !mGoalText.empty() )
  {
    QRect dividerRect(
      static_cast<int>( dc.rect.left ),
      dividerPos,
      static_cast<int>( dc.rect.right - dc.rect.left ),
      1
    );
    if( dividerColor != RGBColor( RGBColor::NullColor ) )
    {
      QColor colorDivider( dividerColor.R(), dividerColor.G(), dividerColor.B() );
      QBrush brushDivider( colorDivider );
      p->fillRect( dividerRect, brushDivider );
    }
  }

  QString line1_ = QString::fromLocal8Bit( line1.c_str() );
  QRect line1Rect(
    static_cast<int>( dc.rect.left ),
    static_cast<int>( dc.rect.top ),
    static_cast<int>( dc.rect.right - dc.rect.left ),
    static_cast<int>( dividerPos - dc.rect.top )
  );
  int size = fm.width( line1_ );
  while( size > line1Rect.width() - 2 * charWidth )
  {
    line1_ = line1_.mid( 2 );
    size = fm.width( line1_ );
  }
  p->drawText( line1Rect, line1_ );

  QString line2_ = QString::fromLocal8Bit( line2.c_str() );
  QRect line2Rect(
    static_cast<int>( dc.rect.left ),
    dividerPos,
    static_cast<int>( dc.rect.right - dc.rect.left ),
    static_cast<int>( dividerPos - dc.rect.top )
  );
  size = fm.width( line2_ );
  while( size > line2Rect.width() - 2 * charWidth )
  {
    line2_ = line2_.mid( 2 );
    size = fm.width( line2_ );
  }
  p->drawText( line2Rect, line2_ );
#endif // __BORLANDC__
}

